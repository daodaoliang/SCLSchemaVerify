#include "xmlschemaverifythread.h"
#include "sclschemaverify.h"

XmlSchemaVerifyThread::XmlSchemaVerifyThread(QObject *parent)
	: QThread(parent)
{
	m_pApp = (SCLSchemaVerify *)parent;
}

XmlSchemaVerifyThread::~XmlSchemaVerifyThread()
{

}

void XmlSchemaVerifyThread::run()
{
	XmlSchema::instance().m_iObjectPos = 0;
	XmlSchema::instance().checkSchema(m_pApp->m_document);
}
