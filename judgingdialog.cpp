/***************************************************************************
    This file is part of Project Lemon
    Copyright (C) 2011 Zhipeng Jia

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
***************************************************************************/
/**
 * judgingdialog.cpp @Project Lemon+
 * Update 2018 Dust1404
 **/
/**
 * judgingdialog.cpp @Project LemonLime
 * Update 2019 iotang
 **/

#include "judgingdialog.h"
#include "ui_judgingdialog.h"
#include "contest.h"
#include "task.h"
#include "judgesharedvariables.h"
#include <QScrollBar>

JudgingDialog::JudgingDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::JudgingDialog)
{
	ui->setupUi(this);
	cursor = new QTextCursor(ui->logViewer->document());
	connect(ui->cancelButton, SIGNAL(clicked()),
	        this, SLOT(stopJudgingSlot()));
	connect(ui->skipButton, SIGNAL(clicked()),
	        this, SLOT(skipJudging()));
}

JudgingDialog::~JudgingDialog()
{
	delete ui;
	delete cursor;
}

void JudgingDialog::setContest(Contest *contest)
{
	curContest = contest;
	connect(curContest, SIGNAL(dialogAlert(QString)),
	        this, SLOT(dialogAlert(QString)));
	connect(curContest, SIGNAL(singleCaseFinished(int, int, int, int, int, int, int)),
	        this, SLOT(singleCaseFinished(int, int, int, int, int, int, int)));
	connect(curContest, SIGNAL(singleSubtaskDependenceFinished(int, int, int)),
	        this, SLOT(singleSubtaskDependenceFinished(int, int, int)));
	connect(curContest, SIGNAL(taskJudgingStarted(QString)),
	        this, SLOT(taskJudgingStarted(QString)));
	connect(curContest, SIGNAL(taskJudgedDisplay(QString, QList<QList<int> >, int)),
	        this, SLOT(taskJudgedDisplay(QString, QList<QList<int> >, int)));
	connect(curContest, SIGNAL(contestantJudgingStart(QString)),
	        this, SLOT(contestantJudgingStart(QString)));
	connect(curContest, SIGNAL(contestantJudgingFinished()),
	        this, SLOT(contestantJudgingFinished()));
	connect(curContest, SIGNAL(contestantJudgedDisplay(QString, int, int)),
	        this, SLOT(contestantJudgedDisplay(QString, int, int)));
	connect(curContest, SIGNAL(compileError(int, int)),
	        this, SLOT(compileError(int, int)));
	connect(this, SIGNAL(stopJudgingSignal()),
	        curContest, SLOT(stopJudgingSlot()));
}

void JudgingDialog::judge(const QStringList &nameList)
{
	stopJudging = false;
	ui->progressBar->setMaximum(curContest->getTotalTimeLimit() * nameList.size());

	for (int i = 0; i < nameList.size(); i ++)
	{
		curContest->judge(nameList[i]);

		if (stopJudging) break;
	}

	/*
	#ifdef Q_OS_LINUX
		QString text = "notify-send --expire-time=2000 --urgency=normal " + tr("Finished") + " \"" + tr("Judge Finished - LemonLime") + "\"";
		QProcess::execute(text);
	#endif
	*/
	accept();
}

void JudgingDialog::judge(const QString &name, int index)
{
	stopJudging = false;
	ui->progressBar->setMaximum(curContest->getTask(index)->getTotalTimeLimit());
	curContest->judge(name, index);

	/*
	#ifdef Q_OS_LINUX
		QString text = "notify-send --expire-time=2000 --urgency=normal " + tr("Finished") + " \"" + tr("Judge Finished - LemonLime") + "\"";
		QProcess::execute(text);
	#endif
	*/
	accept();
}

void JudgingDialog::judge(const QList<QPair<QString, QSet<int> > > &lists)
{
	stopJudging = false;

	int allTime = 0;

	int listsSize = lists.size();

	for (int i = 0; i < listsSize; i++)
	{
		for (QSet<int>::const_iterator j = lists[i].second.begin(); j != lists[i].second.end(); j++)
		{
			allTime += curContest->getTask(*j)->getTotalTimeLimit();
		}
	}

	ui->progressBar->setMaximum(allTime);

	for (int i = 0; i < listsSize; i++)
	{
		curContest->judge(lists[i].first, lists[i].second);

		if (stopJudging) break;
	}
	/*
	#ifdef Q_OS_LINUX
		QString text = "notify-send --expire-time=2000 --urgency=normal " + tr("Finished") + " \"" + tr("Judge Finished - LemonLime") + "\"";
		QProcess::execute(text);
	#endif
	*/
}

void JudgingDialog::judgeAll()
{
	stopJudging = false;
	ui->progressBar->setMaximum(curContest->getTotalTimeLimit() * curContest->getContestantList().size());
	curContest->judgeAll();
	/*
	#ifdef Q_OS_LINUX
		QString text = "notify-send --expire-time=2000 --urgency=normal " + tr("Finished") + " \"" + tr("Judge Finished - LemonLime") + "\"";
		QProcess::execute(text);
	#endif
	*/
	accept();
}

void JudgingDialog::singleCaseFinished(int progress, int x, int y, int result, int scoreGot, int timeUsed, int memoryUsed)
{
	QTextBlockFormat blockFormat;
	blockFormat.setLeftMargin(30);
	cursor->insertBlock(blockFormat);
	QTextCharFormat charFormat, addcharFormat, scorecharFormat;
	charFormat.setFontPointSize(9);
	cursor->insertText(tr("Test case %1.%2: ").arg(x + 1).arg(y + 1), charFormat);
	addcharFormat.setFontPointSize(7);
	addcharFormat.setForeground(QBrush(Qt::darkGray));
	scorecharFormat.setFontPointSize(8);

	QString text, addtext = "", scoretext = "";

	switch (ResultState(result))
	{
		case CorrectAnswer:
			text = tr("Correct answer");
			if (timeUsed >= 0) addtext += tr(" %1 ms").arg(timeUsed);
			if (memoryUsed >= 0) addtext += tr(" %1 MB").arg(1.00 * memoryUsed / 1024.00 / 1024.00);
			if (scoreGot > 0) scoretext = tr("  %1 Pt").arg(scoreGot);
			charFormat.setForeground(QBrush(Qt::darkGreen));
			scorecharFormat.setForeground(QBrush(Qt::darkCyan));
			scorecharFormat.setFontWeight(QFont::Bold);
			break;

		case PartlyCorrect:
			text = tr("Partly correct");
			if (timeUsed >= 0) addtext += tr(" %1 ms").arg(timeUsed);
			if (memoryUsed >= 0) addtext += tr(" %1 MB").arg(1.00 * memoryUsed / 1024.00 / 1024.00);
			if (scoreGot > 0)
			{
				scoretext = tr("  %1 Pt").arg(scoreGot);
				scorecharFormat.setForeground(QBrush(Qt::darkCyan));
				scorecharFormat.setFontWeight(QFont::Bold);
			}
			else
			{
				scoretext = tr("  %1 Pt").arg(abs(scoreGot));
				scorecharFormat.setForeground(QBrush(Qt::darkYellow));
			}
			charFormat.setForeground(QBrush(Qt::darkCyan));
			break;

		case WrongAnswer:
			text = tr("Wrong answer");
			charFormat.setForeground(QBrush(Qt::red));
			break;

		case TimeLimitExceeded:
			text = tr("Time limit exceeded");
			charFormat.setForeground(QBrush(Qt::darkYellow));
			break;

		case MemoryLimitExceeded:
			text = tr("Memory limit exceeded");
			charFormat.setForeground(QBrush(Qt::darkBlue));
			break;

		case RunTimeError:
			text = tr("Run time error");
			charFormat.setForeground(QBrush(Qt::darkMagenta));
			break;

		case Skipped:
			text = tr("Skipped");
			charFormat.setForeground(QBrush(Qt::lightGray));
			break;

		case CannotStartProgram:
			text = tr("Cannot start program");
			charFormat.setForeground(QBrush(Qt::darkRed));
			charFormat.setBackground(QBrush(Qt::lightGray));
			break;

		case FileError:
			text = tr("File error");
			charFormat.setForeground(QBrush(Qt::darkYellow));
			charFormat.setBackground(QBrush(Qt::lightGray));
			break;

		case InteractorError:
			text = tr("Interactor error");
			charFormat.setForeground(QBrush(Qt::white));
			charFormat.setBackground(QBrush(Qt::darkBlue));
			break;

		case InvalidSpecialJudge:
			text = tr("Invalid special judge");
			charFormat.setForeground(QBrush(Qt::white));
			charFormat.setBackground(QBrush(Qt::darkRed));
			break;

		case SpecialJudgeTimeLimitExceeded:
			text = tr("Special judge time limit exceeded");
			charFormat.setForeground(QBrush(Qt::white));
			charFormat.setBackground(QBrush(Qt::darkYellow));
			break;

		case SpecialJudgeRunTimeError:
			text = tr("Special judge run time error");
			charFormat.setForeground(QBrush(Qt::white));
			charFormat.setBackground(QBrush(Qt::darkMagenta));
			break;
	}

	cursor->insertText(text, charFormat);

	if (addtext.length() > 0)cursor->insertText(addtext, addcharFormat);

	if (scoretext.length() > 0)cursor->insertText(scoretext, scorecharFormat);

	ui->progressBar->setValue(ui->progressBar->value() + progress);

	QScrollBar *bar = ui->logViewer->verticalScrollBar();
	bar->setValue(bar->maximum());
}

void JudgingDialog::dialogAlert(QString msg)
{
	QTextBlockFormat blockFormat;
	blockFormat.setLeftMargin(30);
	cursor->insertBlock(blockFormat);
	QTextCharFormat format;
	format.setFontPointSize(9);
	format.setForeground(QBrush(Qt::gray));
	cursor->insertText(msg, format);

	QScrollBar *bar = ui->logViewer->verticalScrollBar();
	bar->setValue(bar->maximum());
}

void JudgingDialog::singleSubtaskDependenceFinished(int x, int y, int status)
{
	QTextBlockFormat blockFormat;
	blockFormat.setLeftMargin(30);
	cursor->insertBlock(blockFormat);
	QTextCharFormat charFormat, ratioFormat;
	charFormat.setFontPointSize(9);
	ratioFormat.setFontPointSize(9);

	QString text;

	if (status >= 2)
	{
		text = QString(tr("Pure"));
		charFormat.setForeground(QBrush(Qt::lightGray));
		ratioFormat.setForeground(QBrush(Qt::green));
	}
	else if (status <= 0)
	{
		text = QString(tr("Lost"));
		charFormat.setForeground(QBrush(Qt::red));
		ratioFormat.setForeground(QBrush(Qt::red));
		ratioFormat.setFontWeight(QFont::Bold);
	}
	else
	{
		text = QString(tr("Far"));
		charFormat.setForeground(QBrush(Qt::lightGray));
		ratioFormat.setForeground(QBrush(Qt::cyan));
	}

	cursor->insertText(tr("Subtask Dependence %1.%2: ").arg(x + 1).arg(y + 1), charFormat);
	cursor->insertText(text, ratioFormat);

	QScrollBar *bar = ui->logViewer->verticalScrollBar();
	bar->setValue(bar->maximum());
}

void JudgingDialog::taskJudgingStarted(const QString &taskName)
{
	QTextBlockFormat blockFormat;
	blockFormat.setLeftMargin(15);
	cursor->insertBlock(blockFormat);
	QTextCharFormat charFormat;
	charFormat.setFontPointSize(10);
	cursor->insertText(tr("Start judging task %1").arg(taskName), charFormat);
	QScrollBar *bar = ui->logViewer->verticalScrollBar();
	bar->setValue(bar->maximum());
}

void JudgingDialog::taskJudgedDisplay(const QString &taskName, const QList< QList<int> > &scoreList, const int mxScore)
{
	QTextBlockFormat blockFormat;
	blockFormat.setLeftMargin(15);
	cursor->insertBlock(blockFormat);
	QTextCharFormat charFormat, scoreFormat;
	charFormat.setFontPointSize(10);
	scoreFormat.setFontPointSize(10);
	scoreFormat.setFontWeight(QFont::Bold);
	scoreFormat.setForeground(QBrush(Qt::darkCyan));

	int allScore = 0;

	for (auto i : scoreList)
	{
		int miScore = 2147483647;

		for (auto j : i)
		{
			if (j >= 0) miScore = qMin(miScore, j);

			if (miScore <= 0)break;
		}

		allScore += miScore;
	}

	cursor->insertText(tr("Score of Task %1 : ").arg(taskName), charFormat);
	cursor->insertText(tr("%1 / %2\n").arg(allScore).arg(mxScore), scoreFormat);
	QScrollBar *bar = ui->logViewer->verticalScrollBar();
	bar->setValue(bar->maximum());
}

void JudgingDialog::contestantJudgingStart(const QString &contestantName)
{
	QTextCharFormat charFormat;
	charFormat.setFontPointSize(12);
	charFormat.setFontWeight(QFont::Bold);
	cursor->insertText(tr("Start judging contestant %1").arg(contestantName), charFormat);
	QScrollBar *bar = ui->logViewer->verticalScrollBar();
	bar->setValue(bar->maximum());
}

void JudgingDialog::contestantJudgingFinished()
{
	QTextBlockFormat blockFormat;
	cursor->insertBlock(blockFormat);
	cursor->insertBlock(blockFormat);
	QScrollBar *bar = ui->logViewer->verticalScrollBar();
	bar->setValue(bar->maximum());
}

void JudgingDialog::contestantJudgedDisplay(const QString &contestantName, const int score, const int mxScore)
{
	QTextBlockFormat blockFormat;
	blockFormat.setLeftMargin(15);
	cursor->insertBlock(blockFormat);
	QTextCharFormat charFormat, scoreFormat;
	charFormat.setFontPointSize(12);
	scoreFormat.setFontPointSize(12);
	scoreFormat.setFontWeight(QFont::Bold);
	scoreFormat.setForeground(QBrush(Qt::darkCyan));

	cursor->insertText(tr("Total score of %1 : ").arg(contestantName), charFormat);
	cursor->insertText(tr("%1 / %2\n").arg(score).arg(mxScore), scoreFormat);
	QScrollBar *bar = ui->logViewer->verticalScrollBar();
	bar->setValue(bar->maximum());
}


void JudgingDialog::compileError(int progress, int compileState)
{
	QTextBlockFormat blockFormat;
	blockFormat.setLeftMargin(30);
	cursor->insertBlock(blockFormat);
	QTextCharFormat charFormat;
	charFormat.setFontPointSize(9);

	QString text;

	switch (CompileState(compileState))
	{
		case NoValidSourceFile:
			text = tr("Cannot find valid source file");
			charFormat.setForeground(QBrush(Qt::white));
			charFormat.setBackground(QBrush(Qt::black));
			break;

		case CompileError:
			text = tr("Compile error");
			charFormat.setForeground(QBrush(Qt::red));
			charFormat.setBackground(QBrush(Qt::black));
			break;

		case CompileTimeLimitExceeded:
			text = tr("Compile time limit exceeded");
			charFormat.setForeground(QBrush(Qt::yellow));
			charFormat.setBackground(QBrush(Qt::black));
			break;

		case InvalidCompiler:
			text = tr("Invalid compiler");
			charFormat.setForeground(QBrush(Qt::magenta));
			charFormat.setBackground(QBrush(Qt::black));
			break;

		case CompileSuccessfully:
			text = tr("Compile Successfully");
			charFormat.setForeground(QBrush(Qt::lightGray));
			break;
	}

	cursor->insertText(text, charFormat);
	ui->progressBar->setValue(ui->progressBar->value() + progress);

	QScrollBar *bar = ui->logViewer->verticalScrollBar();
	bar->setValue(bar->maximum());
}

void JudgingDialog::stopJudgingSlot()
{
	stopJudging = true;
	emit stopJudgingSignal();
}

int skipEnabled;

void JudgingDialog::skipJudging()
{
	skipEnabled = 1;
}

void JudgingDialog::reject()
{
	stopJudgingSlot();
}
