#ifndef XMLSCHEMA_H
#define XMLSCHEMA_H

#include <QtCore>
#include "xmlobject.h"

#define qSafeDelete(p) do { if(p) delete (p); (p)=0; } while(0)

enum eTextLevel
{
	TL_INFO = 1,
	TL_WARN,
	TL_ERROR
};

class XsSimple;
class XsComplex;
class XsSequence;
class XsAttrib
{
public:
	explicit XsAttrib() : simple(0)
	{
	}
	~XsAttrib()
	{
		qSafeDelete(simple);
	}
	QString    name;
	QString    type;
	QString    use;
	XsSimple * simple;
};

class XsRestriction
{
public:
	explicit XsRestriction() : sequence(0)
	{
	}
	~XsRestriction()
	{
		qSafeDelete(sequence);
		qDeleteAll(attributes);
	}
	QString          base;
	QString          name;
	QStringList      pattern;
	QString          minInclusive;
	QStringList      enumeration;
	QString          minLength;
	QString          maxLength;
	XsSequence		 *sequence;
	QList<XsAttrib*> attributes;
};

class XsSimple
{
public:
	explicit XsSimple() : restriction(0)
	{
	}
	~XsSimple()
	{
		qSafeDelete(restriction);
	}

	int				minlen() const;
	int				maxlen() const;
	QStringList		regexp() const;
	QStringList     enumerations();
	QString         name;
	XsRestriction * restriction;
	QStringList     pattern;
	QStringList     unions;
};

class XsAttribGroup
{
public:
	explicit XsAttribGroup()
	{
	}
	~XsAttribGroup()
	{
		qDeleteAll(attributes);
		qDeleteAll(groups);
	}
	QList<XsAttrib*>      allAttributes();
	QString               name;
	QList<XsAttrib*>      attributes;
	QList<XsAttribGroup*> groups;
};

class XsKey
{
public:
	explicit XsKey()
	{
	}
	~XsKey()
	{
	}
	QString        name;
	QString        selector;
	QList<QString> fields;
};

class XsKeyRef
{
public:
	explicit XsKeyRef()
	{
	}
	~XsKeyRef()
	{
	}
	QString        name;
	QString        refer;
	QString        selector;
	QList<QString> fields;
};

class XsUnique
{
public:
	explicit XsUnique()
	{
	}
	~XsUnique()
	{
	}
	QString        name;
	QString        selector;
	QStringList    fields;
};

class XsElement
{
public:
	explicit XsElement() : complex(0), unique(0),simple(0)
	{
	}
	~XsElement()
	{
		qSafeDelete(complex);
		qSafeDelete(unique);
		qSafeDelete(simple);

		qDeleteAll(keys);
		qDeleteAll(keyrefs);
	}
	QString            name;
	QString            type;
	QString            ref;
	QString            minOccurs;
	QString            maxOccurs;
	XsComplex          *complex;
	XsUnique           *unique;
	QList<XsKey*>      keys;
	QList<XsKeyRef*>   keyrefs;
	XsSimple           *simple;
};

class XsAny
{
public:
	explicit XsAny()
	{
	}
	~XsAny()
	{
	}
	QString namesp;
	QString process;
	QString minOccurs;
	QString maxOccurs;
};

class XsAnyAttr
{
public:
	explicit XsAnyAttr()
	{
	}
	~XsAnyAttr()
	{
	}
	QString namesp;
	QString process;
};

class XsAll
{
public:
	explicit XsAll()
	{
	}
	~XsAll()
	{
		qDeleteAll(elements);
	}
	QList<XsElement*> elements;
};

class XsChoice
{
public:
	explicit XsChoice()
	{
	}
	~XsChoice()
	{
		qDeleteAll(elements);
	}
	QString           minOccurs;
	QString           maxOccurs;
	QList<XsElement*> elements;
};

class XsSequence
{
public:
	explicit XsSequence() : any(0), choice(0)
	{
	}
	~XsSequence()
	{
		qSafeDelete(any);
		qSafeDelete(choice);

		qDeleteAll(sequences);
		qDeleteAll(elements);
	}
	XsAny              *any;
	XsChoice		   *choice;
	QList<XsSequence*> sequences;
	QList<XsElement*>  elements;
};

class XsExtension
{
public:
	explicit XsExtension() : sequence(0), choice(0)
	{
	}
	~XsExtension()
	{
		qSafeDelete(sequence);
		qSafeDelete(choice);

		qDeleteAll(attributes);
		qDeleteAll(elements);
	}
	QString           base;
	QList<XsAttrib*>  attributes;
	QStringList       groups;
	XsSequence        *sequence;
	QList<XsElement*> elements;
	XsChoice          *choice;
};

class XsContent
{
public:
	explicit XsContent() : restriction(0),extension(0),sequence(0),choice(0)
	{
	}
	~XsContent()
	{
		qSafeDelete(restriction);
		qSafeDelete(extension);
		qSafeDelete(sequence);
		qSafeDelete(choice);

		qDeleteAll(attributes);
		qDeleteAll(elements);
	}
	QString           mixed;
	XsRestriction	  *restriction;
	XsExtension       *extension;
	QList<XsAttrib*>  attributes;
	QList<QString>    group;
	XsSequence        *sequence;
	QList<XsElement*> elements;
	XsChoice          *choice;
};

class XsComplex
{
public:
	explicit XsComplex() : content(0), sequence(0),anyattr(0), all(0)
	{
	}
	~XsComplex()
	{
		qSafeDelete(content);
		qSafeDelete(sequence);
		qSafeDelete(anyattr);
		qSafeDelete(all);

		qDeleteAll(attribs);
		qDeleteAll(elements);
		qDeleteAll(attrGroups);
	}

	int				  minlen() const;
	int				  maxlen() const;
	QStringList		  regexp() const;
	QList<XsElement*> allElements();
	QList<XsAttrib*>  allAttribs();

	QString name;
	QString abstracted;

	XsContent			*content;
	QList<XsAttrib*>	attribs;

	XsSequence			*sequence;
	XsAnyAttr			*anyattr;
	QList<XsElement*>	elements;

	XsAll					*all;
	QList<XsAttribGroup*>	attrGroups;
};

class XmlSchema : public QObject
{
	Q_OBJECT

public:
	XmlSchema(QObject *parent = 0);
	~XmlSchema();

	quint32 m_iObjectPos;

	static	XmlSchema & instance();
	bool	load(const QString path);
	bool	load(const QStringList list);
	bool	parse(const QString & fileName);
	bool	checkSchema(QList<XmlObject*> & objects);
	bool	xmlObjectValidate(XmlObject * object);

public:
	XsSimple		*simple(const QString & name);
	XsSimple		*simpleWithType(const QString & type);
	XsComplex		*complex(const QString & name);
	XsComplex		*complexWithType(const QString & name);
	XsAttrib		*findAttrib(const XsComplex * complex,const QString & name);
	XsAttribGroup	*findAttribGroup(const QString & name);

private:
	static XmlSchema			*sInstance;
	bool						 schemaValid;
	QString						 schemaFileName;
	QMap<QString,XsSimple*>		 mapSimple;
	QMap<QString,XsAttribGroup*> mapAttribGroup;
	QMap<QString,XsComplex*>     mapComplex;
	QMap<QString,XsElement*>     mapElement;
	QMap<QString,XsAttrib*>      mapAttribute;
	QMap<QString,QString>        mapType;
	QMap<QString,XsUnique*>      mapUnique;

private:
	QString			attr(QXmlStreamReader * reader,const QString & key);
	XsSimple		*readSimple(QXmlStreamReader * reader);
	XsRestriction	*readRestriction(QXmlStreamReader * reader);
	XsAttrib		*readAttribute(QXmlStreamReader * reader);
	XsAttribGroup	*readAttribGroup(QXmlStreamReader * reader);
	XsComplex		*readComplex(QXmlStreamReader * reader);
	XsContent		*readContent(QXmlStreamReader * reader,const char * tag);
	XsSequence		*readSequence(QXmlStreamReader * reader);
	XsElement		*readElement(QXmlStreamReader * reader);
	XsUnique		*readUnique(QXmlStreamReader * reader);
	XsKey			*readKey(QXmlStreamReader * reader);
	XsKeyRef		*readKeyRef(QXmlStreamReader * reader);
	XsChoice		*readChoice(QXmlStreamReader * reader);
	XsAll			*readAll(QXmlStreamReader * reader);
	XsExtension		*readContentExtension(QXmlStreamReader * reader);

private:
	void checkRequired(XsComplex * xc,XmlObject * object);
	void checkSequence(XsComplex * xc,XmlObject * object);
	void checkAttribute(XsComplex * xc,XmlObject * object);
	bool checkAttributeValue(XsAttrib * attr,const QString & value,QString & result);
	bool checkAttributeValueEnumerations(XsSimple * xs,const QString & value,QString & result);
	bool checkAttributeValueRegexp(QStringList patten,const QString & value,QString & result);
	bool checkObjectValue(XmlObject * object,QString value,QString & result);
	bool checkValueLength(XsComplex * xc,QString value,QString & result);
	bool checkValueLength(XsSimple * xs,QString value,QString & result);

signals:
	void throwText(QString text,int level);
	void throwObjectPos(int pos);
	
};

#endif // XMLSCHEMA_H
