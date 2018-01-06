#include "sclschemaverify.h"

SCLSchemaVerify::SCLSchemaVerify(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
{
	ui.setupUi(this);

	Init();
	InitSlot();
}

SCLSchemaVerify::~SCLSchemaVerify()
{

}

void SCLSchemaVerify::Init()
{
	m_iProgressBarPos = 0;
	m_pTimer = new QTimer;
	m_pTimer->setInterval(100);

	ui.radioButton_schema1->setChecked(true);
	ui.radioButton_schema2->setChecked(false);
	ui.radioButton_schema->setChecked(false);
	ui.btnSelectSchema->setEnabled(false);
	ui.btnSchemaVerify->setEnabled(false);
	ui.lineEdit_scl->setReadOnly(true);
	ui.plainTextEdit->setReadOnly(true);
	ui.plainTextEdit->setLineWrapMode(QPlainTextEdit::NoWrap);

	m_pSclParseThread = new XmlSclParseThread(this);
	m_pSchemaParseThread = new XmlSchemaParseThread(this);
	m_pSchemaVerifyThread = new XmlSchemaVerifyThread(this);

	ui.radioButton_schema2->setVisible(true);
}

void SCLSchemaVerify::InitSlot()
{
	connect(ui.radioButton_schema1,SIGNAL(clicked()), this, SLOT(SlotRadioButtonSchema1Clicked()));
	connect(ui.radioButton_schema2,SIGNAL(clicked()), this, SLOT(SlotRadioButtonSchema2Clicked()));
	connect(ui.radioButton_schema,SIGNAL(clicked()), this, SLOT(SlotRadioButtonSchemaClicked()));
	connect(ui.btnSelectSchema,SIGNAL(clicked()), this, SLOT(SlotSelectSchemaFiles()));
	connect(ui.btnSelectScl,SIGNAL(clicked()), this, SLOT(SlotSelectSCLFile()));
	connect(ui.btnSchemaVerify,SIGNAL(clicked()), this, SLOT(SlotSchemaVerify()));
	connect(ui.btnExport,SIGNAL(clicked()), this, SLOT(SlotExport()));
	connect(&XmlSchema::instance(),SIGNAL(throwText(QString,int)), this, SLOT(SlotThrowText(QString,int)));
	connect(&XmlSchema::instance(),SIGNAL(throwObjectPos(int)), this, SLOT(SlotThrowObjectPos(int)));
	connect(m_pSchemaParseThread,SIGNAL(finished()),this,SLOT(SlotSchemaParseThreadFinished()));
	connect(m_pSchemaParseThread,SIGNAL(throwText(QString,int)),this,SLOT(SlotThrowText(QString,int)));
	connect(m_pSchemaVerifyThread,SIGNAL(finished()),this,SLOT(SlotSchemaVerifyThreadFinished()));
	connect(m_pSchemaVerifyThread,SIGNAL(throwText(QString,int)),this,SLOT(SlotThrowText(QString,int)));
	connect(m_pSclParseThread,SIGNAL(finished()),this,SLOT(SlotSclParseThreadFinished()));
	connect(m_pTimer,SIGNAL(timeout()), this, SLOT(SlotTimeout()));
}

void SCLSchemaVerify::SlotRadioButtonSchema1Clicked()
{
	ui.radioButton_schema2->setChecked(false);
	ui.radioButton_schema->setChecked(false);
	ui.btnSelectSchema->setEnabled(false);
}

void SCLSchemaVerify::SlotRadioButtonSchema2Clicked()
{
	ui.radioButton_schema1->setChecked(false);
	ui.radioButton_schema->setChecked(false);
	ui.btnSelectSchema->setEnabled(false);
}

void SCLSchemaVerify::SlotRadioButtonSchemaClicked()
{
	ui.radioButton_schema1->setChecked(false);
	ui.radioButton_schema2->setChecked(false);
	ui.btnSelectSchema->setEnabled(true);
}

void SCLSchemaVerify::SlotSelectSchemaFiles()
{
	QStringList files = QFileDialog::getOpenFileNames(this,tr("ѡ��Schema�ļ�"),QString::null,tr("XSD File(*.xsd)"));
	foreach (QString s, files)
	{
		m_sSchemaFiles.append(s);
	}
}

void SCLSchemaVerify::SlotSelectSCLFile()
{
	m_sSclFile = QFileDialog::getOpenFileName(this,tr("ѡ��SCL�ļ�"),QString::null,tr("SCL File(*.scd *.cid *.icd)"));
	if (m_sSclFile.isEmpty())
		return;

	qDeleteAll(m_document);
	m_document.clear();
	ui.plainTextEdit->clear();
	ui.lineEdit_scl->setText(m_sSclFile);
	m_iProgressBarMax = GetLineNumberByFile(ui.lineEdit_scl->text().trimmed());
	ui.progressBar->setRange(0,m_iProgressBarMax);
	m_pSclParseThread->setSclPath(ui.lineEdit_scl->text().trimmed());
	m_pSclParseThread->start();
	m_pTimer->start();
	SlotThrowText(tr("��ʼ����SCL�ļ�[%1]......").arg(m_sSclFile));
	ui.btnExport->setEnabled(false);
	ui.btnSelectScl->setEnabled(false);
}

void SCLSchemaVerify::SlotSchemaVerify()
{
	m_iWarnNumber = m_iErrorNumber = 0;
	ui.plainTextEdit->clear();
	if (ui.radioButton_schema1->isChecked())
	{
		m_pSchemaParseThread->setSchemaPath(":/schema1");
	}
	else if (ui.radioButton_schema2->isChecked())
	{
		m_pSchemaParseThread->setSchemaPath(":/schema2");
	}
	else if (ui.radioButton_schema->isChecked())
	{
		if (m_sSchemaFiles.count() == 0)
		{
			SlotThrowText(tr("��ѡ���Զ���Schema�ļ���"),TL_WARN);
			return;
		}
		m_pSchemaParseThread->setSchemaFiles(m_sSchemaFiles);
	}

	m_pSchemaParseThread->start();
	ui.btnSelectScl->setEnabled(false);
	ui.btnSchemaVerify->setEnabled(false);
	ui.btnExport->setEnabled(false);
}

void SCLSchemaVerify::SlotExport()
{
	if (m_sSclFile.isEmpty() || ui.plainTextEdit->toPlainText().isEmpty())
	{
		QMessageBox::warning(this,tr("����"),tr("SCL�ļ���У����Ϣ����Ϊ�գ��޷�������"));
		return;
	}

	QFile f(":/file/exportTemplate");
	if(!f.open(QIODevice::ReadOnly | QIODevice::Text)) 
	{    
		QMessageBox::warning(this,tr("����"),tr("����ģ���ȡʧ�ܡ�"));
		return;
	} 

	QFileInfo fi;  
	fi = QFileInfo(m_sSclFile);
	QDateTime dt = QDateTime::currentDateTime();
	QString save = tr("У�鱨��_%1_%2.pdf").arg(fi.fileName()).arg(dt.toString("yyyy��MM��dd��hhʱmm��ss��"));
	QString file = QFileDialog::getSaveFileName(NULL,QObject::tr("У�鱨�浼��"),save,tr("PDF����(*.pdf);;CSV����(*.csv);;TXT����(*.txt)"));
	if (file.isEmpty())
		return;

	QStringList l = ui.plainTextEdit->toPlainText().split("\n");
	QFileInfo fi1;
	fi1 = QFileInfo(file);
	QString suffix = fi1.suffix();
	if (suffix.toLower() == "pdf")
	{
		QByteArray all = f.readAll();
		QString sAll(all);
		sAll.replace("{TITLE}",QString(tr("%1 У�鱨��").arg(fi.fileName())).toStdString().data());
		sAll.replace("{TIME}",dt.toString("yyyy��MM��dd��hhʱmm��ss��"));

		QString content;
		foreach (QString s,l)
		{
			QString font;
			if (s.contains("��ʾ��"))
				font = "font3";
			else if (s.contains("�澯��"))
				font = "font2";
			else if (s.contains("����"))
				font = "font1";
			else
				font = ".font3";
			content += tr("<font class=\"%1\">%2</font><br>").arg(font).arg(s);
		}
		sAll.replace("{CONTENT}",content.toStdString().data());

		QWebView view;
		view.setHtml(sAll);
		QPrinter printer;
		printer.setOutputFormat(QPrinter::PdfFormat);
		printer.setPrintRange(QPrinter::AllPages);
		printer.setOrientation(QPrinter::Portrait);
		printer.setPaperSize(QPrinter::A4);
		printer.setResolution(QPrinter::HighResolution);
		printer.setFullPage(true);
		printer.setOutputFileName(file);
		view.print(&printer);
	}
	else if (suffix.toLower() == "csv" || suffix.toLower() == "txt")
	{
		QFile exportFile(file);
		if (!exportFile.open(QIODevice::WriteOnly|QIODevice::Text))
		{
			QMessageBox::warning(this,tr("��ʾ"),tr("���ļ�[%1]ʧ�ܡ�").arg(file));
			return;
		}

		QTextStream in(&exportFile);
		foreach (QString s,l)
			in << s << "\n";
		exportFile.close();
	}
	else
	{
		QMessageBox::warning(this,tr("��ʾ"),tr("��֧�ֵĵ�����ʽ(%1)��").arg(suffix.toLower()));
		return;
	}

	QMessageBox::information(this,tr("��ʾ"),tr("У�鱨�浼���ɹ���"));
}

void SCLSchemaVerify::SlotThrowText(QString text,int level)
{
	QString msg,color,tip;
	switch (level)
	{
	case TL_INFO:
		color = "#000000";
		tip = tr("��ʾ��");
		break;
	case TL_WARN:
		color = "#0000FF";
		tip = tr("�澯��");
		m_iWarnNumber++;
		break;
	case TL_ERROR:
		color = "#FF0000";
		tip = tr("����");
		m_iErrorNumber++;
		break;
	default:
		color = "#000000";
		tip = tr("δ֪��");
		break;
	}

	msg += tr("<font color=%1>").arg(color);
	msg += tip + text;
	msg += tr("</font>");
	ui.plainTextEdit->appendHtml(msg);
}

void SCLSchemaVerify::SlotThrowObjectPos(int pos)
{
	ui.progressBar->setValue(pos);
}

void SCLSchemaVerify::SlotSchemaParseThreadFinished()
{
	if (!m_pSchemaParseThread->getError().trimmed().isEmpty())
	{
		SlotThrowText(m_pSchemaParseThread->getError().trimmed(),TL_ERROR);
		m_pSchemaParseThread->clearError();
		ui.btnSelectScl->setEnabled(true);
		ui.btnSchemaVerify->setEnabled(true);
		ui.btnExport->setEnabled(true);
		return;
	}

	if (ui.plainTextEdit->toPlainText().contains(tr("δ����")))
	{
		int ret = QMessageBox::question(this,tr("ѯ��"),tr("Schema�ļ���δ�������ݣ��Ƿ���������﷨У�飿"),tr("��"),tr("��"));
		if (ret != 0)
		{
			ui.btnSelectScl->setEnabled(true);
			ui.btnSchemaVerify->setEnabled(true);
			ui.btnExport->setEnabled(true);
			return;
		}
	}

	m_iSclObjectMax = 0;
	GetSclObjectMax(m_document.at(0));
	ui.progressBar->setRange(0,m_iSclObjectMax);
	ui.progressBar->setValue(0);
	SlotThrowText(tr("��ʼSCL�ļ�Schema�﷨У��[%1]......").arg(m_sSclFile));
	m_pSchemaVerifyThread->start();
}

void SCLSchemaVerify::SlotSchemaVerifyThreadFinished()
{
	SlotThrowText(tr("���SCL�ļ�Schema�﷨У�顣���С����󡿹�%1�������澯����%2����").arg(m_iErrorNumber).arg(m_iWarnNumber));
	ui.progressBar->setValue(m_iSclObjectMax);
	ui.btnSelectScl->setEnabled(true);
	ui.btnSchemaVerify->setEnabled(true);
	ui.btnExport->setEnabled(true);
}

void SCLSchemaVerify::SlotSclParseThreadFinished()
{
	ui.btnExport->setEnabled(true);
	ui.btnSelectScl->setEnabled(true);

	if (!m_pSclParseThread->getError().trimmed().isEmpty())
	{
		SlotThrowText(m_pSclParseThread->getError().trimmed(),TL_ERROR);
		m_pSclParseThread->clearError();
	}
	else 
	{
		SlotThrowText(tr("�ɹ�����SCL�ļ������ԡ���ʼSchema�﷨У�顿��"));
		ui.btnSchemaVerify->setEnabled(true);
	}

	m_pTimer->stop();
	ui.progressBar->setValue(m_iProgressBarMax);
	m_iProgressBarPos = 0;
}

void SCLSchemaVerify::SlotTimeout()
{
	ui.progressBar->setValue(m_iProgressBarPos);
}

int	SCLSchemaVerify::GetLineNumberByFile(QString fileName)
{
	int count = 0;

	QFile file(fileName);
	if (!file.open(QFile::ReadOnly | QFile::Text)) 
		return 0;

	QTextStream in(&file);
	while (!in.atEnd()) 
	{
		QString line = in.readLine();
		count++;
	}

	file.close();
	return count;
}

void SCLSchemaVerify::GetSclObjectMax(XmlObject *object)
{
	m_iSclObjectMax++;
	foreach(XmlObject * child,object->children)
		GetSclObjectMax(child);
}

