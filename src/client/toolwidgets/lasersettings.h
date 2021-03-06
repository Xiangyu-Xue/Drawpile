/*
   Drawpile - a collaborative drawing program.

   Copyright (C) 2006-2016 Calle Laakkonen

   Drawpile is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Drawpile is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Drawpile.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef TOOLSETTINGS_LASER_H
#define TOOLSETTINGS_LASER_H

#include "toolsettings.h"

class Ui_LaserSettings;

namespace tools {

class LaserPointerSettings : public QObject, public ToolSettings {
	Q_OBJECT
public:
	LaserPointerSettings(const QString &name, const QString &title, ToolController *ctrl);
	~LaserPointerSettings();

	bool pointerTracking() const;

	void setForeground(const QColor& color);
	void quickAdjust1(float adjustment);

	virtual int getSize() const override { return 0; }
	virtual bool getSubpixelMode() const override { return false; }

	virtual ToolProperties saveToolSettings() override;
	virtual void restoreToolSettings(const ToolProperties &cfg) override;

private slots:
	void updateTool();

signals:
	void pointerTrackingToggled(bool);

protected:
	virtual QWidget *createUiWidget(QWidget *parent);

private:
	Ui_LaserSettings * _ui;
};

}

#endif

