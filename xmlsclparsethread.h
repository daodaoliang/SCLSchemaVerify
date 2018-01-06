#ifndef XMLSCLPARSETHREAD_H
#define XMLSCLPARSETHREAD_H

#include <QThread>

class SCLSchemaVerify;
class XmlSclParseThread : public QThread
{
	Q_OBJECT

public:
	XmlSclParseThread(QObject *parent);
	~XmlSclParseThread();

	void setSclPath(QString path) { m_sSclPath = path; }
	QString getError() { return m_sError; }
	void clearError() { m_sError = QString::null; }

protected:
	virtual void run();

private:
	SCLSchemaVerify *m_pApp;
	QString	m_sSclPath;
	QString m_sError;
	
};

#endif // XMLSCLPARSETHREAD_H
