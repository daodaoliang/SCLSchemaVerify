#include "xmlschemaparsethread.h"
#include "sclschemaverify.h"

XmlSchemaParseThread::XmlSchemaParseThread(QObject *parent)
	: QThread(parent)
{
	m_pApp = (SCLSchemaVerify *)parent;

	m_sSchmeaPath = QString::null;
	m_sSchemaFiles.clear();
}

XmlSchemaParseThread::~XmlSchemaParseThread()
{
	
}

void XmlSchemaParseThread::run()
{
	clearError();
	if (!m_sSchmeaPath.isEmpty())
	{
		QDir dir(m_sSchmeaPath);
		QFileInfoList list = dir.entryInfoList(QDir::Files,QDir::NoSort);
		foreach(QFileInfo fi,list)
		{
			if (!_check(fi.absoluteFilePath()))
				return;
		}

		XmlSchema::instance().load(m_sSchmeaPath);
		m_sSchmeaPath = QString::null;
	}
	else if (m_sSchemaFiles.count() > 0)
	{
		foreach(QString s,m_sSchemaFiles)
		{
			if (!_check(s))
				return;
		}

		XmlSchema::instance().load(m_sSchemaFiles);
		m_sSchemaFiles.clear();
	}
}

bool XmlSchemaParseThread::_check(QString path)
{
	QXmlStreamReader reader;

	throwText(tr("��ʼ����Schema�ļ�[%1]......").arg(path),TL_INFO);
	QFile file(path);
	if (file.open(QFile::ReadOnly | QFile::Text))
	{
		reader.setDevice(&file);
		while(!reader.atEnd())
		{
			QXmlStreamReader::TokenType token = reader.readNext();
			if(reader.hasError())
			{
				m_sError = tr("XML�﷨�����кţ�%1���кţ�%2������%3��").arg(reader.lineNumber()).arg(reader.columnNumber()).arg(reader.errorString());
				return false;
			}
		}
	}
	else
	{
		m_sError = tr("ֻ����ʽ���ļ�[%1]ʧ�ܡ�").arg(path);
		return false;
	}

	throwText(tr("�ɹ�����Schema�ļ�[%1]��").arg(path),TL_INFO);
	return true;
}
