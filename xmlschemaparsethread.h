#ifndef XMLSCHEMAPARSETHREAD_H
#define XMLSCHEMAPARSETHREAD_H

#include <QtCore>
#include <QThread>
#include "xmlschema.h"
#include "xmlobject.h"

class SCLSchemaVerify;
class XmlSchemaParseThread : public QThread
{
	Q_OBJECT

public:
	XmlSchemaParseThread(QObject *parent);
	~XmlSchemaParseThread();

	void setSchemaPath(QString path) { m_sSchmeaPath = path; }
	void setSchemaFiles(QStringList list) { m_sSchemaFiles = list; }
	QString getError() { return m_sError; }
	void clearError() { m_sError = QString::null; }

protected:
	virtual void run();

private:
	SCLSchemaVerify *m_pApp;
	QString	m_sSchmeaPath;
	QStringList m_sSchemaFiles;
	QString m_sError;
	int number;

private:
	bool _check(QString path);

signals:
	void throwText(QString text,int level);
	
};

#endif // XMLSCHEMAPARSETHREAD_H
