#ifndef SCLSCHEMAVERIFY_H
#define SCLSCHEMAVERIFY_H

#include <QtGui/QMainWindow>
#include <QWebView>
#include "ui_sclschemaverify.h"
#include "xmlschema.h"
#include "xmlobjectprivate.h"
#include "xmlschemaparsethread.h"
#include "xmlschemaverifythread.h"
#include "xmlsclparsethread.h"

class SCLSchemaVerify : public QMainWindow
{
	Q_OBJECT

public:
	SCLSchemaVerify(QWidget *parent = 0, Qt::WFlags flags = 0);
	~SCLSchemaVerify();

	QList<XmlObject*> m_document;
	int m_iProgressBarPos;
	int m_iProgressBarMax;

private:
	Ui::SCLSchemaVerifyClass ui;

	QTimer *m_pTimer;
	QString m_sSclFile;
	QStringList m_sSchemaFiles;
	XmlSchemaParseThread *m_pSchemaParseThread;
	XmlSchemaVerifyThread *m_pSchemaVerifyThread;
	XmlSclParseThread *m_pSclParseThread;
	quint32 m_iSclObjectMax;
	quint32 m_iWarnNumber;
	quint32 m_iErrorNumber;

private:
	void Init();
	void InitSlot();
	int	 GetLineNumberByFile(QString fileName);
	void GetSclObjectMax(XmlObject *object);

public slots:
	void SlotRadioButtonSchema1Clicked();
	void SlotRadioButtonSchema2Clicked();
	void SlotRadioButtonSchemaClicked();
	void SlotSelectSchemaFiles();
	void SlotSelectSCLFile();
	void SlotSchemaVerify();
	void SlotExport();
	void SlotThrowText(QString text,int level = TL_INFO);
	void SlotThrowObjectPos(int pos);
	void SlotSchemaParseThreadFinished();
	void SlotSchemaVerifyThreadFinished();
	void SlotSclParseThreadFinished();
	void SlotTimeout();
};

#endif // SCLSCHEMAVERIFY_H
