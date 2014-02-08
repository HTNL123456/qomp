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
#include "qompplaylistmodel.h"
#include "options.h"
#include "qomptrayicon.h"
#include "defines.h"
#include "qompplaylistdelegate.h"
#include "qompmenu.h"
#include "tune.h"

#include "ui_qompmainwin.h"

#include <QCloseEvent>


void QompMainWin::connectMainMenu()
{
	connect(mainMenu_, SIGNAL(actToggleVisibility()), SLOT(toggleVisibility()));
	connect(mainMenu_, SIGNAL(actCheckUpdates()), SIGNAL(checkForUpdates()));
	connect(mainMenu_, SIGNAL(actAbout()), SIGNAL(aboutQomp()));
	connect(mainMenu_, SIGNAL(actDoOptions()), SIGNAL(doOptions()));
	connect(mainMenu_, SIGNAL(tunes(QList<Tune*>)), SIGNAL(tunes(QList<Tune*>)));
	connect(mainMenu_, SIGNAL(actExit()), SIGNAL(exit()));
}

QompMainWin::QompMainWin(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::QompMainWin),
	model_(0),
	trayIcon_(new QompTrayIcon(this)),
	mainMenu_(new QompMainMenu(this)),
	currentState_(Qomp::StateUnknown)
{
	ui->setupUi(this);

	connectActions();
	connectMainMenu();
	setIcons();
	setupPlaylist();

	ui->tb_repeatAll->setChecked(Options::instance()->getOption(OPTION_REPEAT_ALL).toBool());

	trayIcon_->setContextMenu(mainMenu_);

	connect(ui->seekSlider, SIGNAL(valueChanged(int)), SIGNAL(seekSliderMoved(int)));
	connect(ui->volumeSlider, SIGNAL(valueChanged(int)), SLOT(volumeSliderMoved(int)));
	connect(this, SIGNAL(customContextMenuRequested(QPoint)), SLOT(doMainContextMenu()));
	//connect(trayIcon_, SIGNAL(trayContextMenu()), SLOT(doMainContextMenu()));
	connect(trayIcon_, SIGNAL(trayWheeled(int)), SLOT(trayWheeled(int)));

	restoreWindowState();
}

QompMainWin::~QompMainWin()
{
	saveWindowState();

	delete ui;
}

void QompMainWin::setModel(QompPlayListModel *model)
{
	model_ = model;
	ui->playList->setModel(model_);
	connect(model_, SIGNAL(currentTuneChanged(Tune*)), SLOT(updateTuneInfo(Tune*)));
}

void QompMainWin::bringToFront()
{
	if(isMaximized())
		showMaximized();
	else
		show();
	raise();
	activateWindow();
}

void QompMainWin::setMuteState(bool mute)
{
	ui->tb_mute->setChecked(mute);
	ui->tb_mute->setIcon(ui->tb_mute->isChecked() ?
				     style()->standardIcon(QStyle::SP_MediaVolumeMuted) :
				     style()->standardIcon(QStyle::SP_MediaVolume));
}

void QompMainWin::updateIcons(Qomp::State state)
{
	switch (state) {
	case Qomp::StatePaused: {
		ui->tb_play->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
		static const QIcon pauseIco(":/icons/icons/qomp_pause.png");
		trayIcon_->setIcon(pauseIco);
		break;
	}
	case Qomp::StatePlaying: {
		ui->tb_play->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
		static const QIcon playIcon(":/icons/icons/qomp_play.png");
		trayIcon_->setIcon(playIcon);
		break;
	}
	default: {
		static const QIcon stopIco(":/icons/icons/qomp_stop.png");
		trayIcon_->setIcon(stopIco);
		ui->tb_play->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
		break;
	}
	}
}

void QompMainWin::updateOptions()
{
	Options::instance()->setOption(OPTION_REPEAT_ALL, ui->tb_repeatAll->isChecked());
}

void QompMainWin::actOpenActivated()
{
	QompGetTunesMenu m;
	connect(&m, SIGNAL(tunes(QList<Tune*>)), SIGNAL(tunes(QList<Tune*>)));

	m.exec(QCursor::pos());
}

void QompMainWin::removeSelectedIndexes()
{
	emit removeSelected(ui->playList->selectionModel()->selectedIndexes());
}

void QompMainWin::actClearActivated()
{
	QompRemoveTunesMenu m(this);
	connect(&m, SIGNAL(removeAll()), SIGNAL(clearPlaylist()));
	connect(&m, SIGNAL(removeSelected()), SLOT(removeSelectedIndexes()));

	m.exec(QCursor::pos());

	updateTuneInfo(model_->currentTune());
}

void QompMainWin::volumeSliderMoved(int vol)
{
	qreal newVol = qreal(vol)/1000;
	emit volumeSliderMoved(newVol);
}

void QompMainWin::volumeChanged(qreal vol)
{
	ui->volumeSlider->blockSignals(true);
	ui->volumeSlider->setValue(vol*1000);
	ui->volumeSlider->blockSignals(false);
}

void QompMainWin::doTrackContextMenu(const QPoint &p)
{
	QModelIndex index = ui->playList->indexAt(p);
	if(!index.isValid())
		return;

	ui->playList->setCurrentIndex(index);
	Tune* tune = model_->tune(index);
	QompTrackMenu menu(tune, this);
	connect(&menu, SIGNAL(togglePlayState(Tune*)), SIGNAL(toggleTuneState(Tune*)));
	connect(&menu, SIGNAL(saveTune(Tune*)), SIGNAL(downloadTune(Tune*)));
	connect(&menu, SIGNAL(removeTune(Tune*)), SIGNAL(removeTune(Tune*)));

	menu.exec(QCursor::pos());
}

void QompMainWin::doMainContextMenu()
{
	mainMenu_->exec(QCursor::pos());
}

void QompMainWin::playerStateChanged(Qomp::State state)
{
	if(currentState_ == state)
		return;

	updateIcons(state);
	currentState_ = state;

	switch(state) {
	case Qomp::StateBuffering:
		ui->lb_busy->changeText(tr("Buffering"));
		ui->lb_busy->start();
		break;
	case Qomp::StateLoading:
		ui->lb_busy->changeText(tr("Loading"));
		ui->lb_busy->start();
		break;
	default:
		ui->lb_busy->stop();
		break;
	}
	updateTuneInfo(model_->currentTune());
}

void QompMainWin::setCurrentPosition(qint64 ms)
{
	ui->lcd->display(Qomp::durationMiliSecondsToString(ms));
	ui->seekSlider->blockSignals(true);
	ui->seekSlider->setValue(ms);
	ui->seekSlider->blockSignals(false);
}

void QompMainWin::currentTotalTimeChanged(qint64 ms)
{
	ui->seekSlider->setMaximum(ms);

	if(ms == -1 || ms == 0)
		return;
	model_->currentTune()->duration = Qomp::durationMiliSecondsToString(ms);
	model_->tuneDataUpdated(model_->currentTune());
}

void QompMainWin::toggleVisibility()
{
	bool b = isHidden();
	if(b) {
		bringToFront();
	}
	else
		hide();
}

void QompMainWin::trayWheeled(int delta)
{
	int vol = ui->volumeSlider->value();
	vol += delta;
	emit volumeChanged(qreal(vol)/1000);
}

void QompMainWin::closeEvent(QCloseEvent *e)
{
	if(Options::instance()->getOption(OPTION_HIDE_ON_CLOSE).toBool()) {
		hide();
		e->ignore();
	}
	else {
		emit exit();
		e->accept();
	}
}

void QompMainWin::changeEvent(QEvent *e)
{
	if(e->type() == QEvent::LanguageChange) {
		ui->retranslateUi(this);
	}
	QMainWindow::changeEvent(e);
}

void QompMainWin::connectActions()
{
	connect(ui->tb_repeatAll, SIGNAL(clicked()), SLOT(updateOptions()));
	connect(ui->tb_open, SIGNAL(clicked()), SLOT(actOpenActivated()));
	connect(ui->tb_play, SIGNAL(clicked()), SIGNAL(actPlayActivated()));
	connect(ui->tb_stop, SIGNAL(clicked()), SIGNAL(actStopActivated()));
	connect(ui->tb_clear, SIGNAL(clicked()), SLOT(actClearActivated()));
	connect(ui->tb_next, SIGNAL(clicked()), SIGNAL(actNextActivated()));
	connect(ui->tb_prev, SIGNAL(clicked()), SIGNAL(actPrevActivated()));
	connect(ui->tb_load, SIGNAL(clicked()), SIGNAL(loadPlaylist()));
	connect(ui->tb_save, SIGNAL(clicked()), SIGNAL(savePlaylist()));
	connect(ui->tb_mute, SIGNAL(clicked(bool)), SIGNAL(actMuteActivated(bool)));
}

void QompMainWin::setIcons()
{
	ui->tb_next->setIcon(style()->standardIcon(QStyle::SP_MediaSeekForward));
	ui->tb_prev->setIcon(style()->standardIcon(QStyle::SP_MediaSeekBackward));
	ui->tb_play->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
	ui->tb_stop->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
	ui->tb_clear->setIcon(style()->standardIcon(QStyle::SP_DialogResetButton));
	ui->tb_load->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
	ui->tb_save->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
	ui->tb_open->setIcon(style()->standardIcon(QStyle::SP_DriveCDIcon));
	ui->tb_mute->setIcon(style()->standardIcon(QStyle::SP_MediaVolume));
	ui->tb_repeatAll->setIcon((style()->standardIcon(QStyle::SP_BrowserReload)));
}

void QompMainWin::setupPlaylist()
{
	ui->playList->setItemDelegate(new QompPlaylistDelegate(this));

	connect(ui->playList, SIGNAL(activated(QModelIndex)), SIGNAL(mediaActivated(QModelIndex)));
	connect(ui->playList, SIGNAL(clicked(QModelIndex)), SIGNAL(mediaClicked(QModelIndex)));
	connect(ui->playList, SIGNAL(customContextMenuRequested(QPoint)), SLOT(doTrackContextMenu(QPoint)));
}

void QompMainWin::saveWindowState()
{
	Options::instance()->setOption(OPTION_GEOMETRY_X, x());
	Options::instance()->setOption(OPTION_GEOMETRY_Y, y());
	Options::instance()->setOption(OPTION_GEOMETRY_HEIGHT, height());
	Options::instance()->setOption(OPTION_GEOMETRY_WIDTH, width());
}

void QompMainWin::restoreWindowState()
{
	resize(Options::instance()->getOption(OPTION_GEOMETRY_WIDTH, width()).toInt(),
		Options::instance()->getOption(OPTION_GEOMETRY_HEIGHT, height()).toInt());

	move(Options::instance()->getOption(OPTION_GEOMETRY_X, 10).toInt(),
		Options::instance()->getOption(OPTION_GEOMETRY_Y, 50).toInt());
}

void QompMainWin::updateTuneInfo(Tune* tune)
{
	QModelIndex i = model_->indexForTune(tune);
	if(i.isValid() && (currentState_ == Qomp::StatePaused || currentState_ == Qomp::StatePlaying)) {
		ui->lb_artist->setText(i.data(QompPlayListModel::ArtistRole).toString());
		ui->lb_title->setText(i.data(QompPlayListModel::TitleRole).toString());

		trayIcon_->setToolTip(tr("Now playing: %1 - %2")
				      .arg(i.data(QompPlayListModel::ArtistRole).toString(),
					   i.data(QompPlayListModel::TitleRole).toString()));
	} else {
		ui->lb_artist->setText("");
		ui->lb_title->setText("");
		trayIcon_->setToolTip("");
	}
}
