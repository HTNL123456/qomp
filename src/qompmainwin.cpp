/*
 * Copyright (C) 2013  Khryukin Evgeny
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "qompmainwin.h"
#include "qompplayer.h"
#include "playlistmodel.h"
#include "options.h"
#include "pluginmanager.h"
#include "qomptrayicon.h"
#include "options/qompoptionsdlg.h"
#include "defines.h"

#include "ui_qompmainwin.h"

#include <QTime>
#include <QNetworkAccessManager>
#include <QCloseEvent>
#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>

static const QString cachedPlayListFileName = "/qomp-cached-playlist.qomp";


QompMainWin::QompMainWin(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::QompMainWin),
	nam_(new QNetworkAccessManager(this))
{
	ui->setupUi(this);

	player_ = new QompPlayer(this);
	player_->setSeekSlider(ui->seekSlider);
	player_->setVolumeSlider(ui->volumeSlider);

	connect(player_, SIGNAL(currentPosition(qint64)), SLOT(setCurrentPosition(qint64)));

	model_ = new PlayListModel(this);
	ui->playList->setModel(model_);

	TuneList tl = Tune::tunesFromFile(QDesktopServices::storageLocation(QDesktopServices::CacheLocation) + cachedPlayListFileName);
	model_->addTunes(tl);

	ui->tb_next->setIcon(style()->standardIcon(QStyle::SP_MediaSeekForward));
	ui->tb_prev->setIcon(style()->standardIcon(QStyle::SP_MediaSeekBackward));
	ui->tb_play->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
	ui->tb_stop->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
	ui->tb_clear->setIcon(style()->standardIcon(QStyle::SP_DialogResetButton));
	ui->tb_load->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
	ui->tb_save->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
	ui->tb_open->setIcon(style()->standardIcon(QStyle::SP_DriveCDIcon));

	setCurrentPosition(0);

	connect(ui->tb_open, SIGNAL(clicked()), SLOT(actOpenActivated()));
	connect(ui->tb_play, SIGNAL(clicked()), SLOT(actPlayActivated()));
	connect(ui->tb_stop, SIGNAL(clicked()), SLOT(actStopActivated()));
	connect(ui->tb_clear, SIGNAL(clicked()), SLOT(actClearActivated()));
	connect(ui->tb_next, SIGNAL(clicked()), SLOT(actNextActivated()));
	connect(ui->tb_prev, SIGNAL(clicked()), SLOT(actPrevActivated()));
	connect(ui->tb_load, SIGNAL(clicked()), SLOT(loadPlaylist()));
	connect(ui->tb_save, SIGNAL(clicked()), SLOT(savePlaylist()));

	connect(ui->actionOpen, SIGNAL(triggered()), SLOT(actOpenActivated()));
	connect(ui->actionExit, SIGNAL(triggered()), SIGNAL(exit()));

	connect(ui->playList, SIGNAL(activated(QModelIndex)), SLOT(mediaActivated(QModelIndex)));
	connect(ui->playList, SIGNAL(clicked(QModelIndex)), SLOT(mediaClicked(QModelIndex)));
	connect(player_, SIGNAL(stateChanged(Phonon::State,Phonon::State)), SLOT(updatePlayIcon()));
	connect(player_, SIGNAL(mediaFinished()), SLOT(playNext()));

	QompTrayIcon* ico = new QompTrayIcon(this);
	connect(ico, SIGNAL(trayDoubleClicked()), SLOT(trayDoubleclicked()));
	connect(ico, SIGNAL(trayClicked(Qt::MouseButton)), SLOT(trayActivated(Qt::MouseButton)));

	if(Options::instance()->getOption(OPTION_AUTOSTART_PLAYBACK).toBool())
		actPlayActivated();
}

QompMainWin::~QompMainWin()
{
	QDir dir = QDesktopServices::storageLocation(QDesktopServices::CacheLocation);
	if(!dir.exists())
		dir.mkpath(dir.path());

	savePlaylist(QDesktopServices::storageLocation(QDesktopServices::CacheLocation) + cachedPlayListFileName);

	delete ui;
}

void QompMainWin::actPlayActivated()
{
	if(!model_->rowCount())
		return;

	QModelIndex i = model_->indexForTune(model_->currentTune());
	if(!i.isValid()) {
		i = model_->index(0);
		ui->playList->setCurrentIndex(i);
		model_->setCurrentTune(model_->tune(i));
	}

	if(!(player_->currentSource() == model_->device(i)))
		player_->setSource(model_->device(i));

	player_->play();
	updateTuneInfoFrame();
}

void QompMainWin::updatePlayIcon()
{
	if(player_->state() == Phonon::PlayingState)
		ui->tb_play->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
	else
		ui->tb_play->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
}

void QompMainWin::actPrevActivated()
{
	QModelIndex index = ui->playList->currentIndex();
	if(index.isValid() && index.row() > 0) {
		bool play = (player_->state() == Phonon::PlayingState);
		index = model_->index(index.row()-1);
		ui->playList->setCurrentIndex(index);
		model_->setCurrentTune(model_->tune(index));
		if(play)
			actPlayActivated();
	}
}

void QompMainWin::actNextActivated()
{
	QModelIndex index = ui->playList->currentIndex();
	if(index.isValid() && index.row() < model_->rowCount()-1) {
		bool play = (player_->state() == Phonon::PlayingState);
		index = model_->index(index.row()+1);
		ui->playList->setCurrentIndex(index);
		model_->setCurrentTune(model_->tune(index));
		if(play)
			actPlayActivated();

	}
}

void QompMainWin::actStopActivated()
{
	player_->stop();
	QModelIndex index = ui->playList->currentIndex();
	if(index.isValid())
		model_->setCurrentTune(model_->tune(index));
	ui->lb_artist->setText("");
	ui->lb_title->setText("");
}

void QompMainWin::actOpenActivated()
{
	TuneList list;
	QMenu m;
	QList<QAction*> acts;
	foreach(const QString& name, PluginManager::instance()->availablePlugins()) {
		acts.append(new QAction(name, &m));
	}
	m.addActions(acts);
	m.move(QCursor::pos());
	QAction* x = m.exec();
	if(x)
		list = PluginManager::instance()->getTune(x->text());


	if(!list.isEmpty()) {
		model_->addTunes(list);
		if(player_->state() == Phonon::StoppedState) {
			QModelIndex index = model_->indexForTune(list.first());
			ui->playList->setCurrentIndex(index);
			model_->setCurrentTune(model_->tune(index));
			actPlayActivated();
		}
	}
}

void QompMainWin::actClearActivated()
{
	player_->stop();
	model_->clear();
	updateTuneInfoFrame();
}

void QompMainWin::mediaActivated(const QModelIndex &index)
{
	model_->setCurrentTune(model_->tune(index));
	actPlayActivated();
}

void QompMainWin::mediaClicked(const QModelIndex &index)
{
	if(player_->state() == Phonon::StoppedState) {
		model_->setCurrentTune(model_->tune(index));
	}
}

void QompMainWin::setCurrentPosition(qint64 ms)
{
	int h = ms / (1000*60*60);
	ms -= h*(1000*60*60);
	int m = ms / (1000*60);
	ms -= m*(1000*60);
	int s = ms / 1000;
	ms -= s*1000;
	QTime t(h, m, s, ms);
	const QString format = h ? "hh:mm:ss" : "mm:ss";
	ui->lcd->display(t.toString(format));
}

void QompMainWin::playNext()
{
	if(model_->indexForTune(model_->currentTune()).row() == model_->rowCount()-1) {
		actStopActivated();
	}
	else {
		QModelIndex index = model_->indexForTune(model_->currentTune());
		index = model_->index(index.row()+1);
		ui->playList->setCurrentIndex(index);
		model_->setCurrentTune(model_->tune(index));
		actPlayActivated();
	}
}

void QompMainWin::doOptions()
{
	QompOptionsDlg dlg;
	dlg.exec();
}

void QompMainWin::loadPlaylist()
{
	QString file = QFileDialog::getOpenFileName(this, tr("Select Playlist"),
						    Options::instance()->getOption(LAST_DIR, QDir::homePath()).toString(), "*qomp");
	if(!file.isEmpty()) {
		Options::instance()->setOption(LAST_DIR, file);
		TuneList tl = Tune::tunesFromFile(file);
		model_->addTunes(tl);
	}
}

void QompMainWin::savePlaylist()
{
	QString file = QFileDialog::getSaveFileName(this, tr("Save Playlist"),
						    Options::instance()->getOption(LAST_DIR, QDir::homePath()).toString(), "*qomp");
	if(!file.isEmpty()) {
		Options::instance()->setOption(LAST_DIR, file);
		savePlaylist(file);
	}
}

void QompMainWin::trayDoubleclicked()
{
	bool b = isHidden();
	if(b)
		show();
	else
		hide();
}

void QompMainWin::trayActivated(Qt::MouseButton b)
{
	if(b == Qt::RightButton) {
		QMenu m;
		QList<QAction*> acts;
		if(isHidden()) {
			acts << new QAction(tr("Show"), &m);
		}
		else {
			acts << new QAction(tr("Hide"), &m);
		}
		acts << new QAction(tr("Open"), &m);
		acts << new QAction(tr("Settings"), &m);
		QAction* sep = new QAction(&m);
		sep->setSeparator(true);
		acts << sep;
		acts << new QAction(tr("Exit"), &m);
		m.addActions(acts);
		m.move(QCursor::pos());
		int ret = acts.indexOf(m.exec());
		if(ret == 0)
			trayDoubleclicked();
		else if(ret == 1)
			actOpenActivated();
		else if(ret == 2)
			doOptions();
		else if(ret == 4)
			emit exit();
	}
}

void QompMainWin::closeEvent(QCloseEvent *e)
{
	hide();
	e->ignore();
}

void QompMainWin::savePlaylist(const QString &fileName)
{
	QString f(fileName);
	if(!f.endsWith(".qomp"))
		f += ".qomp";
	QFile file(f);
	if(file.open(QFile::ReadWrite | QFile::Truncate)) {
		if(model_->rowCount() > 0) {
			QTextStream ts(&file);
			ts.setCodec("UTF-8");
			for(int i = 0; i < model_->rowCount(); i++) {
				QString str = model_->tune(model_->index(i)).toString();
				ts << str << endl;
			}
		}
	}
}

void QompMainWin::updateTuneInfoFrame()
{
	QModelIndex i = model_->indexForTune(model_->currentTune());
	ui->lb_artist->setText(i.data(PlayListModel::ArtistRole).toString());
	ui->lb_title->setText(i.data(PlayListModel::TitleRole).toString());
}