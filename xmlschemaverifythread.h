#ifndef XMLSCHEMAVERIFYTHREAD_H
#define XMLSCHEMAVERIFYTHREAD_H

#include <QtCore>
#include <QThread>
#include "xmlschema.h"

class SCLSchemaVerify;
class XmlSchemaVerifyThread : public QThread
{
	Q_OBJECT

public:
	XmlSchemaVerifyThread(QObject *parent);
	~XmlSchemaVerifyThread();

protected:
	virtual void run();

private:
	SCLSchemaVerify *m_pApp;

signals:
	void throwText(QString text,int level);
	
};

#endif // XMLSCHEMAVERIFYTHREAD_H
