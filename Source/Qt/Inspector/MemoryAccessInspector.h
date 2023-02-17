#pragma once
// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Memory.h"
#include "MemoryInspector.h"
#include "Templates/Array.h"
class QComboBox;


namespace gui
{
class GWidgetRGB;

class MemoryAccessInspector : public MemoryInspector
{
	GWidgetRGB* graphics_view;
	QComboBox*	combobox_bytes_per_row; // 32, 64, 128, 256, 512
	QComboBox*	combobox_pixelzoom;		// 2x2, 3x3, 4x4   <=>   pixel_size = 3, 4, 5
	QComboBox*	combobox_decaymode;
	enum { modeFlash, modeDecayFast, modeDecaySlow, modeAccumulate };

	Array<QRgb> rom_pixels;
	Array<QRgb> ram_pixels;

	int decay_mode; // combobox_mode
	int pixel_size; // combobox_zoom: 3 .. 5 incl. space between pixels

public:
	MemoryAccessInspector(QWidget* parent, MachineController* mc, volatile IsaObject*);
	~MemoryAccessInspector();

protected:
	void resizeEvent(QResizeEvent*) override;
	// void		wheelEvent(QWheelEvent*) override;
	// void		paintEvent(QPaintEvent*) override;
	// void		mousePressEvent(QMouseEvent*) override;
	// void		mouseMoveEvent(QMouseEvent*) override;
	// bool		event(QEvent*) override;
	// void		keyPressEvent(QKeyEvent*) override;
	// void		keyReleaseEvent(QKeyEvent*) override;
	// void		showEvent(QShowEvent*) override;
	// void		hideEvent(QHideEvent*) override;

	void saveSettings() override;
	void adjustSize(QSize&) override;
	// void		adjustMaxSizeDuringResize() override;

	void updateWidgets() override; // timer

	// void		slotSetMemoryPage(int) override;	// combobox_memorypage
	// void		setScrollPosition(int) override;	// scrollbar
	// void		slotSetDataSource(int) override;	// combobox_datasource
	void slotMemoryConfigChanged(Memory*, uint how) override;

private:
	int	 width_for_bytes(int n);
	int	 height_for_rows(int n);
	int	 rows_for_height(int h);
	int	 bytes_for_width(int w);
	void decay_pixel(Array<QRgb>&, int);
	void copy_access_bits_to_pixels(MemoryPtr, Array<QRgb>&);
	void prepare_for_fast_decay(Array<QRgb>&);
	void clear_pixel(Array<QRgb>&);
	void save_settings();
	void validate_rows();
	void validate_bytes_per_row();
	void validate_scrollposition();
	void updateTooltip();
	void slotSetDecayMode(int);	  // combobox_decaymode
	void slotSetPixelSize(int);	  // combobox_pixelzoom
	void slotSetBytesPerRow(int); // combobox_bytes_per_row
};

} // namespace gui
