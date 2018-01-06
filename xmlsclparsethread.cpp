#include "xmlsclparsethread.h"
#include "sclschemaverify.h"

XmlSclParseThread::XmlSclParseThread(QObject *parent)
	: QThread(parent)
{
	m_pApp = (SCLSchemaVerify *)parent;

	m_sSclPath = QString::null;
	m_sError = QString::null;
}

XmlSclParseThread::~XmlSclParseThread()
{

}

void XmlSclParseThread::run()
{
	clearError();
	if (m_sSclPath.isEmpty())
	{
		m_sError = tr("未发现SCL文件，请选择SCL文件。");
		return;
	}

	QXmlStreamReader reader;
	QFile file(m_sSclPath);
	if (file.open(QFile::ReadOnly | QFile::Text))
	{
		reader.setDevice(&file);

		XmlObject * parent  = 0;
		XmlObject * current = 0;
		while(!reader.atEnd())
		{
			m_pApp->m_iProgressBarPos = reader.lineNumber();

			QXmlStreamReader::TokenType token = reader.readNext();
			if(reader.isStartElement())
			{
				current = new XmlObject();
				current->parent = parent;
				current->name   = reader.qualifiedName().toString();
				current->line   = reader.lineNumber();

				if(parent)
					parent->children.append(current);
				else
					m_pApp->m_document.append(current);

				if(current->name == "SCL")
				{
					current->attributes.insert("xsi:schemaLocation","http://www.iec.ch/61850/2003/SCL SCL.xsd");
					current->attributes.insert("xmlns","http://www.iec.ch/61850/2003/SCL");
					current->attributes.insert("xmlns:xsi","http://www.w3.org/2001/XMLSchema-instance");
					//current->attributes.insert("xmlns:csg","http://www.csg.cn");
				}
				else
				{
					QXmlStreamAttributes attributes = reader.attributes();
					for(int i = 0; i < attributes.count(); i++)
					{
						QXmlStreamAttribute attrib = attributes.at(i);
						current->attributes.insert(attrib.qualifiedName().toString(),attrib.value().toString());
					}
				}

				parent = current;
			}
			else if(reader.isEndElement())
			{
				parent = parent->parent;
			}
			else if(token == QXmlStreamReader::Characters)
			{
				if(parent)
				{
					QString text = reader.text().toString().trimmed();
					if(!text.isEmpty())
					{
						parent->text = reader.text().toString();
						parent->attributes.insert("value",text);
					}
				}
			}

			if(reader.hasError())
			{
				m_sError = tr("XML语法错误！行号：%1，列号：%2，错误：%3。").arg(reader.lineNumber()).arg(reader.columnNumber()).arg(reader.errorString());
				qDeleteAll(m_pApp->m_document);
				m_pApp->m_document.clear();
				break;
			}
		}	
	}
	else
	{
		m_sError = tr("只读方式打开文件[%1]失败。").arg(m_sSclPath);
	}

	reader.clear();
	file.close();
}
