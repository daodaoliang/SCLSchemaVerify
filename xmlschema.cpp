#include "xmlschema.h"

#define XSD_SIMPLE_TYPE "simpleType"
#define XSD_BLANK_SPACE " "

static bool showPrompt(const QString & tag)
{
	static QStringList list = QString("schema,annotation,documentation,include").split(",",QString::SkipEmptyParts);

	if (list.contains(tag)) 
		return false;

	return true;
}

QStringList XsSimple::enumerations()
{
	QStringList list;

	if (restriction) 
		list += restriction->enumeration;

	foreach (QString type,unions)
	{
		XsSimple * simple = XmlSchema::instance().simpleWithType(type);
		if (simple) 
			list += simple->enumerations();
	}

	return list;
}

QStringList XsSimple::regexp() const
{
	QStringList list;
	
	if(restriction)
	{
		list += restriction->pattern;
		XsSimple * xs = XmlSchema::instance().simpleWithType(restriction->base);
		if(xs)
			list += xs->regexp();
	}

	foreach (QString type,unions)
	{
		XsSimple * simple = XmlSchema::instance().simpleWithType(type);
		if (simple) 
			list += simple->regexp();
	}

	QStringList tList;
	foreach (QString s, list)
	{
		s.replace("\\p{Lu}","[A-Z]");
		s.replace("\\p{L},","A-Za-z,");
		s.replace(",\\p{L}",",A-Za-z");
		s.replace("\\p{L}","[A-Za-z]");
		s.replace("\\p{Ll}","[a-z]");
		tList += s;
	}
	
	return tList;
}

int XsSimple::minlen() const
{
	bool ret;
	int len = -1;

	if (restriction)
	{
		int min = restriction->minLength.toInt(&ret);
		if (ret && (min > len))
			len = min;

		XsSimple * xs = XmlSchema::instance().simpleWithType(restriction->base);
		if(xs)
		{
			int min = xs->minlen();
			if (min > len)
				len = min;
		}
	}

	return len;
}

int XsSimple::maxlen() const
{
	bool ret;
	int len = -1;

	if (restriction)
	{
		int max = restriction->maxLength.toInt(&ret);
		if (ret && (max > len))
			len = max;

		XsSimple * xs = XmlSchema::instance().simpleWithType(restriction->base);
		if(xs)
		{
			int max = xs->maxlen();
			if (max > len)
				len = max;
		}
	}

	return len;
}

QList<XsAttrib*> XsAttribGroup::allAttributes()
{
	QList<XsAttrib*> list;

	XsAttribGroup * group = XmlSchema::instance().findAttribGroup(name);
	if (group)
	{
		list += group->attributes;

		foreach (XsAttribGroup * child,group->groups)
		{
			list += child->allAttributes();
		}
	}

	return list;
}

QList<XsElement*> XsComplex::allElements()
{
	QList<XsElement*> list;
	list += elements;

	if (sequence) 
		list += sequence->elements;

	if (content)
	{
		list += content->elements;

		if (content->sequence) 
			list += content->sequence->elements;

		XsRestriction * restriction = content->restriction;
		if (restriction)
		{
			XsComplex * complex = XmlSchema::instance().complexWithType(restriction->base);
			if (complex) 
				list += complex->allElements();
		}

		XsExtension * extension = content->extension;
		if (extension)
		{
			list += content->extension->elements;

			if (content->extension->choice)   
				list += content->extension->choice->elements;

			if (content->extension->sequence) 
			{
				list += content->extension->sequence->elements;
				if (content->extension->sequence->choice) 
					list += content->extension->sequence->choice->elements;
			}

			XsComplex * complex = XmlSchema::instance().complexWithType(extension->base);
			if (complex) 
				list += complex->allElements();
		}
	}

	if (all) 
		list += all->elements;

	return list;
}

QList<XsAttrib*> XsComplex::allAttribs()
{
	QList<XsAttrib*> list;

	list += attribs;

	if (content)
	{
		list += content->attributes;
		XsExtension * extension = content->extension;
		if (extension)
		{
			list += extension->attributes;

			foreach (QString groupName,extension->groups)
			{
				XsAttribGroup * group = XmlSchema::instance().findAttribGroup(groupName);
				if (group) 
					list += group->allAttributes();
			}

			XsComplex * complex = XmlSchema::instance().complexWithType(extension->base);
			if (complex) 
				list += complex->allAttribs();
		}

		if (content->restriction) 
		{
			list += content->restriction->attributes;

			XsComplex * complex = XmlSchema::instance().complexWithType(content->restriction->base);
			if (complex) 
				list += complex->allAttribs();
		}
	}
	foreach (XsAttribGroup * groups,attrGroups)
	{
		XsAttribGroup * group = XmlSchema::instance().findAttribGroup(groups->name);
		if (group) 
			list += group->allAttributes();
	}

	return list;
}

QStringList XsComplex::regexp() const
{
	QStringList list;

	if(content && content->restriction)
	{
		list += content->restriction->pattern;
		XsSimple * xs = XmlSchema::instance().simpleWithType(content->restriction->base);
		if(xs)
			list += xs->regexp();
	}

	QStringList tList;
	foreach (QString s, list)
	{
		s.replace("\\p{Lu}","[A-Z]");
		s.replace("\\p{L},","A-Za-z,");
		s.replace(",\\p{L}",",A-Za-z");
		s.replace("\\p{L}","[A-Za-z]");
		s.replace("\\p{Ll}","[a-z]");
		tList += s;
	}

	return tList;
}

int XsComplex::minlen() const
{
	bool ret;
	int len = -1;

	if (content && content->restriction)
	{
		int min = content->restriction->minLength.toInt(&ret);
		if (ret && (min > len))
			len = min;

		XsComplex * xc = XmlSchema::instance().complexWithType(content->restriction->base);
		if(xc)
		{
			int min = xc->minlen();
			if (min > len)
				len = min;
		}
		else
		{
			XsSimple * xs = XmlSchema::instance().simpleWithType(content->restriction->base);
			if (xs)
			{
				int min = xs->minlen();
				if (min > len)
					len = min;
			}
		}
	}

	return len;
}

int XsComplex::maxlen() const
{
	bool ret;
	int len = -1;

	if (content && content->restriction)
	{
		int max = content->restriction->maxLength.toInt(&ret);
		if (ret && (max > len))
			len = max;

		XsComplex * xc = XmlSchema::instance().complexWithType(content->restriction->base);
		if(xc)
		{
			int max = xc->maxlen();
			if (max > len)
				len = max;
		}
		else
		{
			XsSimple * xs = XmlSchema::instance().simpleWithType(content->restriction->base);
			if (xs)
			{
				int max = xs->maxlen();
				if (max > len)
					len = max;
			}
		}
	}

	return len;
}

XmlSchema * XmlSchema::sInstance(0);
XmlSchema & XmlSchema::instance()
{
	if (!sInstance)
	{
		sInstance = new XmlSchema;
		Q_CHECK_PTR(sInstance);
	}

	return *sInstance;
}

XmlSchema::XmlSchema(QObject *parent)
	: QObject(parent)
{

}

XmlSchema::~XmlSchema()
{
	sInstance = 0;
}

bool XmlSchema::load(const QString path)
{
	QDir dir(path);
	QFileInfoList list = dir.entryInfoList(QDir::Files,QDir::NoSort);

	foreach (QFileInfo fi,list)
	{
		if (!parse(fi.absoluteFilePath()))
			return false;
	}

	throwText(tr("加载Schema文件完毕。").arg(schemaFileName),TL_INFO);
	return true;
}

bool XmlSchema::load(const QStringList list)
{
	foreach (QString s,list)
	{
		if (!parse(s))
			return false;
	}

	throwText(tr("加载Schema文件完毕。").arg(schemaFileName),TL_INFO);
	return true;
}

bool XmlSchema::parse(const QString & fileName)
{
	schemaFileName = fileName;
	QFile file(schemaFileName);

	if (!file.open(QFile::ReadOnly | QFile::Text)) 
	{
		throwText(tr("打开文件[%1]失败.").arg(schemaFileName),TL_ERROR);
		return false;
	}

	throwText(tr("加载文件[%1]......").arg(schemaFileName),TL_INFO);
	QXmlStreamReader reader(&file);
	while(!reader.atEnd())
	{
		if (!reader.readNextStartElement()) 
			continue;

		if (reader.prefix().toString() != "xs") 
			continue;

		QString name = reader.name().toString();
		if (name == "simpleType")
		{
			XsSimple * simple = readSimple(&reader);
			mapSimple.insert(simple->name,simple);
		}
		else if (name == "attributeGroup")
		{
			XsAttribGroup * attribGroup = readAttribGroup(&reader);
			mapAttribGroup.insert(attribGroup->name,attribGroup);
		}
		else if (name == "complexType")
		{
			XsComplex * complex = readComplex(&reader);
			mapComplex.insert(complex->name,complex);
		}
		else if (name == "element")
		{
			XsElement * element = readElement(&reader);
			mapElement.insert(element->name,element);
		}
		else if (name == "attribute")
		{
			XsAttrib *attrib = readAttribute(&reader);
			mapAttribute.insert(attrib->name,attrib);
		}
		else if (showPrompt(name)) 
			throwText(tr("XSD文件[%1]中行号[%2]，变量[%3]未处理！").arg(schemaFileName).arg(reader.lineNumber()).arg(name),TL_WARN);
	}

	file.close();
	return true;
}

QString XmlSchema::attr(QXmlStreamReader * reader,const QString & key)
{
	QXmlStreamAttributes attributes = reader->attributes();

	for(int i = 0; i < attributes.count();i++)
	{
		QXmlStreamAttribute attrib = attributes.at(i);
		if (attrib.name().toString() == key)
			return attrib.value().toString();
	}

	return QString::null;
}

XsSimple * XmlSchema::readSimple(QXmlStreamReader * reader)
{
	XsSimple * simple = new XsSimple();

	simple->name = attr(reader,"name");

	while(true)
	{
		reader->readNext();

		if (reader->isStartElement())
		{
			QString name = reader->name().toString();
			if (name == "union")
			{
				simple->unions = attr(reader,"memberTypes").split(XSD_BLANK_SPACE,QString::SkipEmptyParts);
			}
			else if (name == "restriction")
			{
				simple->restriction = readRestriction(reader);
			}
			else if (showPrompt(name)) 
				throwText(tr("XSD文件[%1]中行号[%2]，变量[%3]未处理！").arg(schemaFileName).arg(reader->lineNumber()).arg(name),TL_WARN);
		}
		if (reader->isEndElement() && reader->name().toString() == ("simpleType")) 
			break;
	}

	return simple;
}

XsRestriction * XmlSchema::readRestriction(QXmlStreamReader * reader)
{
	XsRestriction * restriction = new XsRestriction();

	restriction->base = attr(reader,"base");

	while(true)
	{
		reader->readNext();

		if (reader->isStartElement())
		{
			QString name = reader->name().toString();
			if (name == "enumeration")
			{
				restriction->enumeration += attr(reader,"value");
			}
			else if (name == "pattern")
			{
				restriction->pattern += attr(reader,"value");
			}
			else if (name == "minLength")
			{
				restriction->minLength = attr(reader,"value");
			}
			else if (name == "maxLength")
			{
				restriction->maxLength = attr(reader,"value");
			}
			else if (name == "minInclusive")
			{
				restriction->minInclusive = attr(reader,"value");
			}
			else if (name == "attribute")
			{
				restriction->attributes += readAttribute(reader);
			}
			else if (name == "sequence")
			{
				restriction->sequence = readSequence(reader);
			}
			else if (showPrompt(name)) 
				throwText(tr("XSD文件[%1]中行号[%2]，变量[%3]未处理！").arg(schemaFileName).arg(reader->lineNumber()).arg(name),TL_WARN);
		}
		if (reader->isEndElement() && reader->name().toString() == ("restriction")) 
			break;
	}

	return restriction;
}

XsAttrib * XmlSchema::readAttribute(QXmlStreamReader * reader)
{
	XsAttrib * attrib = new XsAttrib();

	attrib->name = attr(reader,"name");
	attrib->type = attr(reader,"type");
	attrib->use  = attr(reader,"use");

	while(true)
	{
		reader->readNext();

		if (reader->isStartElement())
		{
			QString name = reader->name().toString();
			if (name == "simpleType")
			{
				attrib->simple = readSimple(reader);
			}
			else if (showPrompt(name)) 
				throwText(tr("XSD文件[%1]中行号[%2]，变量[%3]未处理！").arg(schemaFileName).arg(reader->lineNumber()).arg(name),TL_WARN);
		}
		if (reader->isEndElement() && reader->name().toString() == ("attribute")) 
			break;
	}

	return attrib;
}

XsAttribGroup * XmlSchema::readAttribGroup(QXmlStreamReader * reader)
{
	XsAttribGroup * attribGroup = new XsAttribGroup;
	attribGroup->name = attr(reader,"name");
	if (attribGroup->name.isEmpty())
		attribGroup->name = attr(reader,"ref");

	while(true)
	{
		reader->readNext();

		if (reader->isStartElement())
		{
			QString name = reader->name().toString();
			if (name == "attribute")
			{
				attribGroup->attributes += readAttribute(reader);
			}
			else if (name == "attributeGroup")
			{
				attribGroup->groups += readAttribGroup(reader);
				continue;
			}
			else if (showPrompt(name)) 
				throwText(tr("XSD文件[%1]中行号[%2]，变量[%3]未处理！").arg(schemaFileName).arg(reader->lineNumber()).arg(name),TL_WARN);
		}
		if (reader->isEndElement() && reader->name().toString() == ("attributeGroup")) 
			break;
	}

	return attribGroup;
}

XsComplex * XmlSchema::readComplex(QXmlStreamReader * reader)
{
	XsComplex * complex = new XsComplex;
	complex->name       = attr(reader,"name");
	complex->abstracted = attr(reader,"abstract");

	while(true)
	{
		reader->readNext();

		if (reader->isStartElement())
		{
			QString name = reader->name().toString();
			if (name == "complexContent")
			{
				complex->content = readContent(reader,"complexContent");
			}
			else if (name == "simpleContent")
			{
				complex->content = readContent(reader,"simpleContent");
			}
			else if (name == "attribute")
			{
				complex->attribs += readAttribute(reader);
			}
			else if (name == "sequence")
			{
				complex->sequence = readSequence(reader);
			}
			else if (name == "anyAttribute")
			{
				complex->anyattr = new XsAnyAttr();
				complex->anyattr->namesp  = attr(reader,"namespace");
				complex->anyattr->process = attr(reader,"processContents");
			}
			else if (name == "element")
			{
				complex->elements += readElement(reader);
			}
			else if (name == "all")
			{
				complex->all = readAll(reader);
			}
			else if (name == "attributeGroup")
			{
				complex->attrGroups += readAttribGroup(reader);
			}
			else if (showPrompt(name)) 
				throwText(tr("XSD文件[%1]中行号[%2]，变量[%3]未处理！").arg(schemaFileName).arg(reader->lineNumber()).arg(name),TL_WARN);
		}
		if (reader->isEndElement() && reader->name().toString() == ("complexType")) 
			break;
	}

	return complex;
}

XsContent * XmlSchema::readContent(QXmlStreamReader * reader,const char * tag)
{
	XsContent * content = new XsContent();
	content->mixed = attr(reader,"mixed");

	while(true)
	{
		reader->readNext();

		if (reader->isStartElement())
		{
			QString name = reader->name().toString();
			if (name == "extension")
			{
				content->extension = readContentExtension(reader);
			}
			else if (name == "attribute")
			{
				content->attributes += readAttribute(reader);
			}
			else if (name == "restriction")
			{
				content->restriction = readRestriction(reader);
			}
			else if (showPrompt(name)) 
				throwText(tr("XSD文件[%1]中行号[%2]，变量[%3]未处理！").arg(schemaFileName).arg(reader->lineNumber()).arg(name),TL_WARN);
		}
		if (reader->isEndElement() && reader->name().toString() == (tag)) 
			break;
	}

	return content;
}

XsExtension * XmlSchema::readContentExtension(QXmlStreamReader * reader)
{
	XsExtension * extension = new XsExtension();
	extension->base = attr(reader,"base");

	while(true)
	{
		reader->readNext();

		if (reader->isStartElement())
		{
			QString name = reader->name().toString();
			if (name == "attribute")
			{
				extension->attributes += readAttribute(reader);
			}
			else if (name == "attributeGroup")
			{
				extension->groups += attr(reader,"ref");
			}
			else if (name == "sequence")
			{
				extension->sequence = readSequence(reader);
			}
			else if (name == "element")
			{
				extension->elements += readElement(reader);
			}
			else if (name == "choice")
			{
				extension->choice = readChoice(reader);
			}
			else if (showPrompt(name)) 
				throwText(tr("XSD文件[%1]中行号[%2]，变量[%3]未处理！").arg(schemaFileName).arg(reader->lineNumber()).arg(name),TL_WARN);
		}
		if (reader->isEndElement() && reader->name().toString() == ("extension")) 
			break;
	}

	return extension;
}

XsSequence * XmlSchema::readSequence(QXmlStreamReader * reader)
{
	XsSequence * sequence = new XsSequence();
	while(true)
	{
		reader->readNext();

		if (reader->isStartElement())
		{
			QString name = reader->name().toString();
			if (name == "element")
			{
				sequence->elements += readElement(reader);
			}
			else if (name == "choice")
			{
				sequence->choice = readChoice(reader);
			}
			else if (name == "sequence")
			{
				sequence->sequences += readSequence(reader);
			}
			else if (name == "any")
			{
				sequence->any = new XsAny();
				sequence->any->namesp = attr(reader,"namespace");
				sequence->any->process = attr(reader,"processContents");
				sequence->any->minOccurs = attr(reader,"minOccurs");
				sequence->any->maxOccurs = attr(reader,"maxOccurs");
			}
			else if (showPrompt(name)) 
				throwText(tr("XSD文件[%1]中行号[%2]，变量[%3]未处理！").arg(schemaFileName).arg(reader->lineNumber()).arg(name),TL_WARN);
		}
		if (reader->isEndElement() && reader->name().toString() == ("sequence")) 
			break;
	}

	return sequence;
}

XsElement * XmlSchema::readElement(QXmlStreamReader * reader)
{
	XsElement * element = new XsElement();

	element->name      = attr(reader,"name");
	element->ref       = attr(reader,"ref");
	element->type      = attr(reader,"type");
	element->minOccurs = attr(reader,"minOccurs");
	element->maxOccurs = attr(reader,"maxOccurs");

	if ((!element->name.isEmpty()) &&(!element->type.isEmpty()))
	{
		mapType.insert(element->name,element->type);
	}

	while(true)
	{
		reader->readNext();

		if (reader->isStartElement())
		{
			QString name = reader->name().toString();
			if (name == "complexType")
			{
				XsComplex * complex = readComplex(reader);

				if (complex->name.isEmpty())
				{
					complex->name = QString("t%1").arg(element->name);
				}
				element->complex = complex;
				mapComplex.insert(complex->name,complex);
			}
			else if (name == "unique")
			{
				element->unique = readUnique(reader);
			}
			else if (name == "key")
			{
				element->keys += readKey(reader);
			}
			else if (name == "keyref")
			{
				element->keyrefs += readKeyRef(reader);
			}
			else if (name == "simpleType")
			{
				element->simple = readSimple(reader);
			}
			else if (showPrompt(name)) 
				throwText(tr("XSD文件[%1]中行号[%2]，变量[%3]未处理！").arg(schemaFileName).arg(reader->lineNumber()).arg(name),TL_WARN);
		}
		if (reader->isEndElement() && reader->name().toString() == ("element")) 
			break;
	}

	return element;
}

XsUnique * XmlSchema::readUnique(QXmlStreamReader * reader)
{
	XsUnique * unique = new XsUnique();

	unique->name   = attr(reader,"name");
	mapUnique.insert(unique->name,unique);

	while(true)
	{
		reader->readNext();

		if (reader->isStartElement())
		{
			QString name = reader->name().toString();
			if (name == "selector")
			{
				unique->selector = attr(reader,"xpath");
			}
			else if (name == "field")
			{
				unique->fields += attr(reader,"xpath").replace("@","");
			}
			else if (showPrompt(name)) 
				throwText(tr("XSD文件[%1]中行号[%2]，变量[%3]未处理！").arg(schemaFileName).arg(reader->lineNumber()).arg(name),TL_WARN);
		}
		if (reader->isEndElement() && reader->name().toString() == ("unique")) 
			break;
	}

	return unique;
}

XsKey  * XmlSchema::readKey(QXmlStreamReader * reader)
{
	XsKey * key = new XsKey();

	key->name = attr(reader,"name");

	while(true)
	{
		reader->readNext();

		if (reader->isStartElement())
		{
			QString name = reader->name().toString();
			if (name == "selector")
			{
				key->selector = attr(reader,"xpath");
			}
			else if (name == "field")
			{
				key->fields += attr(reader,"xpath");
			}
			else if (showPrompt(name)) 
				throwText(tr("XSD文件[%1]中行号[%2]，变量[%3]未处理！").arg(schemaFileName).arg(reader->lineNumber()).arg(name),TL_WARN);
		}
		if (reader->isEndElement() && reader->name().toString() == ("key")) 
			break;
	}

	return key;
}

XsKeyRef * XmlSchema::readKeyRef(QXmlStreamReader * reader)
{
	XsKeyRef * keyref = new XsKeyRef();

	keyref->name   = attr(reader,"name");
	keyref->refer  = attr(reader,"refer");

	while(true)
	{
		reader->readNext();

		if (reader->isStartElement())
		{
			QString name = reader->name().toString();
			if (name == "selector")
			{
				keyref->selector = attr(reader,"xpath");
			}
			else if (name == "field")
			{
				keyref->fields += attr(reader,"xpath");
			}
			else if (showPrompt(name)) 
				throwText(tr("XSD文件[%1]中行号[%2]，变量[%3]未处理！").arg(schemaFileName).arg(reader->lineNumber()).arg(name),TL_WARN);
		}
		if (reader->isEndElement() && reader->name().toString() == ("keyref")) 
			break;
	}

	return keyref;
}

XsChoice * XmlSchema::readChoice(QXmlStreamReader * reader)
{
	XsChoice * choice = new XsChoice();

	choice->minOccurs  = attr(reader,"minOccurs");
	choice->maxOccurs = attr(reader,"maxOccurs");

	while(true)
	{
		reader->readNext();

		if (reader->isStartElement())
		{
			QString name = reader->name().toString();
			if (name == "element")
			{
				choice->elements += readElement(reader);
			}
			else if (showPrompt(name)) 
				throwText(tr("XSD文件[%1]中行号[%2]，变量[%3]未处理！").arg(schemaFileName).arg(reader->lineNumber()).arg(name),TL_WARN);
		}
		if (reader->isEndElement() && reader->name().toString() == ("choice")) 
			break;
	}

	return choice;
}

XsAll * XmlSchema::readAll(QXmlStreamReader * reader)
{
	XsAll * all = new XsAll();

	while(true)
	{
		reader->readNext();

		if (reader->isStartElement())
		{
			QString name = reader->name().toString();
			if (name == "element")
			{
				all->elements += readElement(reader);
			}
			else if (showPrompt(name)) 
				throwText(tr("XSD文件[%1]中行号[%2]，变量[%3]未处理！").arg(schemaFileName).arg(reader->lineNumber()).arg(name),TL_WARN);
		}
		if (reader->isEndElement() && reader->name().toString() == ("all")) 
			break;
	}

	return all;
}

bool XmlSchema::checkSchema(QList<XmlObject*> & objects)
{
	schemaValid = true;

	foreach (XmlObject * object,objects)
		xmlObjectValidate(object);

	return schemaValid;
}

bool XmlSchema::xmlObjectValidate(XmlObject * object)
{
	m_iObjectPos++;
	if (!(m_iObjectPos % 100))
		throwObjectPos(m_iObjectPos);

	if (!object) 
		return false;

	XsComplex * xc = complex(object->name);
	if (xc)
	{
		checkRequired(xc,object);
		checkAttribute(xc,object);
		checkSequence(xc,object);
	}

	foreach (XmlObject * child,object->children)
	{
		xmlObjectValidate(child);
	}

	return true;
}

void XmlSchema::checkRequired(XsComplex * xc,XmlObject * object)
{
	QString result = QString::null;

	QList<XsAttrib*> attrs = xc->allAttribs();
	foreach (XsAttrib *attr, attrs)
	{
		if (attr->use == "required" && !object->attributes.contains(attr->name))
		{
			result += tr("%1,").arg(attr->name);
		}
	}

	if (!result.isEmpty())
	{
		result = result.left(result.size()-1);
		throwText(tr("SCL行号[%1]：对象[%2]缺少属性项[%3]。")
			.arg(object->line).arg(object->name).arg(result),TL_ERROR);
	}
}

void XmlSchema::checkAttribute(XsComplex * xc,XmlObject * object)
{
	bool found = false;
	QList<XsAttrib*> attrs = xc->allAttribs();

	QMap<QString,QString>::iterator iter = object->attributes.begin();
	for(;iter != object->attributes.end();iter++)
	{
		if (iter.key() == "value") 
		{
			QString result;
			if (!checkObjectValue(object,iter.value(),result))
			{
				throwText(tr("SCL行号[%1]：对象[%2]的数值[%3]，在schema文件校验中不合规。原因：[%4]。")
					.arg(object->line).arg(object->name).arg(iter.value()).arg(result),TL_ERROR);
			}
			continue;
		}

		found = false;
		foreach (XsAttrib * attr,attrs)
		{
			if (attr->name == iter.key())
			{
				found = true;
				QString result;
				if (!checkAttributeValue(attr,iter.value(),result))
				{
					throwText(tr("SCL行号[%1]：对象[%2]中属性[%3]的数值[%4]，在schema文件校验中不合规。原因：[%5]。")
						.arg(object->line).arg(object->name).arg(iter.key()).arg(iter.value()).arg(result),TL_ERROR);
				}
				break;
			}
		}
		if (!found)
		{
			schemaValid = false;
			throwText(tr("SCL行号[%1]：对象[%2]中属性[%3]，在schema文件校验中无引用。").arg(object->line).arg(object->name).arg(iter.key()),TL_ERROR);
		}
	}
}

bool XmlSchema::checkObjectValue(XmlObject * object,QString value,QString & result)
{
	if (value.isEmpty())
		return true;

	if (object->name == "P")
	{
		QString type = "tP_"+object->attrib("type");
		XsComplex * xc = mapComplex[type];
		if (xc)
		{
			QStringList patten;
			patten += xc->regexp();

			QList<XsAttrib*> attrs = xc->allAttribs();
			foreach (XsAttrib * attr,attrs)
			{
				XsSimple * xs = simpleWithType(attr->type);
				patten += xs->regexp();
				if (!checkAttributeValueRegexp(patten,value,result))
				{
					return false;
				}
			}

			if (!checkValueLength(xc,value,result))
				return false;
		}
	}

	return true;
}

bool XmlSchema::checkAttributeValue(XsAttrib * attr,const QString & value,QString & result)
{
	bool ret = false;
	if (!attr) 
		return ret;

	XsSimple * xs = simpleWithType(attr->type);
	if (xs)
	{
		ret = checkAttributeValueEnumerations(xs,value,result);
	}
	else
	{
		if (attr->type.isEmpty() && attr->simple)
		{
			ret = checkAttributeValueEnumerations(attr->simple,value,result);
		}
		else if (attr->type.left(4) == "scl:")
		{
			xs = simpleWithType(attr->type.right(attr->type.length()-4));
			if (xs)
				ret = checkAttributeValueEnumerations(xs,value,result);
		}
		else
		{
			if (attr->type == "xs:integer")
			{
				value.toInt(&ret);
				if (!ret)
					result = tr("数值不是有符号整形数");
			}
			else if (attr->type == "xs:unsignedInt")
			{
				value.toUInt(&ret);
				if (!ret)
					result = tr("数值不是无符号整形数");
			}
			else if (attr->type == "xs:boolean")
			{
				ret = (bool)((value == "true") || (value == "false"));
				if (!ret)
					result = tr("数值不是布尔型数");
			}
			else if (attr->type == "xs:normalizedString")
				ret = true;
			else
				result = tr("数值类型[%1]无法处理").arg(attr->type);
		}
	}

	return ret;
}

bool XmlSchema::checkAttributeValueEnumerations(XsSimple * xs,const QString & value,QString & result)
{
	bool ret = false;

	if (!checkValueLength(xs,value,result))
		return false;

	QStringList enums = xs->enumerations();
	if (enums.count() > 0)
	{
		ret = enums.contains(value);
		if (!ret)
		{
			QStringList patten = xs->regexp();
			if (patten.count() > 0)
				ret = checkAttributeValueRegexp(patten,value,result);
			else
				result = tr("在其对应的有效数据列表中未发现此数据");
		}
	}
	else
	{
		QStringList patten = xs->regexp();
		if (patten.count() > 0)
			ret = checkAttributeValueRegexp(patten,value,result);
		else
			ret = true;
	}

	return ret;
}

bool XmlSchema::checkAttributeValueRegexp(QStringList patten,const QString & value,QString & result)
{
	QString tmp;
	bool match = false;
	foreach (QString p, patten)
	{
		QRegExp rx(p);
		if (rx.exactMatch(value))
		{
			match = true;
			break;
		}
		if (!p.isEmpty())
			tmp += p + "::";
	}
	if (!match)
	{
		result = tr("数值与对应的Schema数值模式不匹配，模式为::%1").arg(tmp);
		return false;
	}

	return true;
}

bool XmlSchema::checkValueLength(XsComplex * xc,QString value,QString & result)
{
	int len = xc->minlen();
	if (len >= 0 && value.size() < len)
	{
		result = tr("数值长度小于Schema校验最小值%1").arg(len);
		return false;
	}
	len = xc->maxlen();
	if (len >= 0 && value.size() > len)
	{
		result = tr("数值长度大于Schema校验最大值%1").arg(len);
		return false;
	}

	return true;
}

bool XmlSchema::checkValueLength(XsSimple * xs,QString value,QString & result)
{
	int len = xs->minlen();
	if (len >= 0 && value.size() < len)
	{
		result = tr("数值长度小于Schema校验最小值%1").arg(len);
		return false;
	}
	len = xs->maxlen();
	if (len >= 0 && value.size() > len)
	{
		result = tr("数值长度大于Schema校验最大值%1").arg(len);
		return false;
	}

	return true;
}

void XmlSchema::checkSequence(XsComplex * xc,XmlObject * object)
{
	bool found = false;
	QList<XsElement*> elements = xc->allElements();
	foreach (XmlObject * child,object->children)
	{
		found = false;
		foreach (XsElement * e,elements)
		{
			if (e->name == child->name || e->ref == child->name)
			{
				found = true;
				break;
			}
		}
		if (!found)
		{
			schemaValid = false;
			throwText(tr("SCL行号[%1]：对象[%2]中子项[%3]，在schema文件校验中无引用。").arg(child->line).arg(object->name).arg(child->name),TL_ERROR);
		}
	}
}

XsSimple * XmlSchema::simple(const QString & name)
{
	XsSimple * s = 0;

	if(!mapType.contains(name)) 
		return s;

	QString type = mapType[name];

	if(mapSimple.contains(type))
	{
		s = mapSimple[type];
	}

	return s;
}

XsSimple * XmlSchema::simpleWithType(const QString & type)
{
	XsSimple * s = 0;

	if (mapSimple.contains(type))
	{
		s = mapSimple[type];
	}

	return s;
}

XsComplex * XmlSchema::complex(const QString & name)
{
	QString type;

	XsComplex * c = 0;

	if (!mapType.contains(name))
		type = QString("t%1").arg(name);
	else
		type = mapType[name];

	if (mapComplex.contains(type))
	{
		c = mapComplex[type];
	}

	return c;
}

XsComplex * XmlSchema::complexWithType(const QString & type)
{
	XsComplex * c = 0;

	if (mapComplex.contains(type))
	{
		c = mapComplex[type];
	}

	return c;
}

XsAttribGroup * XmlSchema::findAttribGroup(const QString & name)
{
	if (!mapAttribGroup.contains(name)) 
		return 0;

	return mapAttribGroup[name];
}

XsAttrib * XmlSchema::findAttrib(const XsComplex * complex,const QString & name)
{
	XsContent * content = complex->content;

	if (!content) 
		return 0;

	foreach (XsAttrib * a,content->attributes)
	{
		if (a->name == name)
		{
			return a;
		}
	}

	if (!content->restriction) 
		return 0;

	foreach (XsAttrib * a,content->restriction->attributes)
	{
		if (a->name == name)
		{
			return a;
		}
	}

	return 0;
}
