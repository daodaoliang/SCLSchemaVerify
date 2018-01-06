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
	QStringList files = QFileDialog::getOpenFileNames(this,tr("选择Schema文件"),QString::null,tr("XSD File(*.xsd)"));
	foreach (QString s, files)
	{
		m_sSchemaFiles.append(s);
	}
}

void SCLSchemaVerify::SlotSelectSCLFile()
{
	m_sSclFile = QFileDialog::getOpenFileName(this,tr("选择SCL文件"),QString::null,tr("SCL File(*.scd *.cid *.icd)"));
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
	SlotThrowText(tr("开始解析SCL文件[%1]......").arg(m_sSclFile));
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
			SlotThrowText(tr("请选择自定义Schema文件。"),TL_WARN);
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
		QMessageBox::warning(this,tr("警告"),tr("SCL文件或校验信息内容为空，无法导出。"));
		return;
	}

	QFile f(":/file/exportTemplate");
	if(!f.open(QIODevice::ReadOnly | QIODevice::Text)) 
	{    
		QMessageBox::warning(this,tr("警告"),tr("导出模板读取失败。"));
		return;
	} 

	QFileInfo fi;  
	fi = QFileInfo(m_sSclFile);
	QDateTime dt = QDateTime::currentDateTime();
	QString save = tr("校验报告_%1_%2.pdf").arg(fi.fileName()).arg(dt.toString("yyyy年MM月dd日hh时mm分ss秒"));
	QString file = QFileDialog::getSaveFileName(NULL,QObject::tr("校验报告导出"),save,tr("PDF报告(*.pdf);;CSV报告(*.csv);;TXT报告(*.txt)"));
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
		sAll.replace("{TITLE}",QString(tr("%1 校验报告").arg(fi.fileName())).toStdString().data());
		sAll.replace("{TIME}",dt.toString("yyyy年MM月dd日hh时mm分ss秒"));

		QString content;
		foreach (QString s,l)
		{
			QString font;
			if (s.contains("提示："))
				font = "font3";
			else if (s.contains("告警："))
				font = "font2";
			else if (s.contains("错误："))
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
			QMessageBox::warning(this,tr("提示"),tr("打开文件[%1]失败。").arg(file));
			return;
		}

		QTextStream in(&exportFile);
		foreach (QString s,l)
			in << s << "\n";
		exportFile.close();
	}
	else
	{
		QMessageBox::warning(this,tr("提示"),tr("不支持的导出格式(%1)。").arg(suffix.toLower()));
		return;
	}

	QMessageBox::information(this,tr("提示"),tr("校验报告导出成功。"));
}

void SCLSchemaVerify::SlotThrowText(QString text,int level)
{
	QString msg,color,tip;
	switch (level)
	{
	case TL_INFO:
		color = "#000000";
		tip = tr("提示：");
		break;
	case TL_WARN:
		color = "#0000FF";
		tip = tr("告警：");
		m_iWarnNumber++;
		break;
	case TL_ERROR:
		color = "#FF0000";
		tip = tr("错误：");
		m_iErrorNumber++;
		break;
	default:
		color = "#000000";
		tip = tr("未知：");
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

	if (ui.plainTextEdit->toPlainText().contains(tr("未处理")))
	{
		int ret = QMessageBox::question(this,tr("询问"),tr("Schema文件有未处理内容，是否继续进行语法校验？"),tr("是"),tr("否"));
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
	SlotThrowText(tr("开始SCL文件Schema语法校验[%1]......").arg(m_sSclFile));
	m_pSchemaVerifyThread->start();
}

void SCLSchemaVerify::SlotSchemaVerifyThreadFinished()
{
	SlotThrowText(tr("完成SCL文件Schema语法校验。其中【错误】共%1条，【告警】共%2条。").arg(m_iErrorNumber).arg(m_iWarnNumber));
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
		SlotThrowText(tr("成功解析SCL文件，可以【开始Schema语法校验】。"));
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

