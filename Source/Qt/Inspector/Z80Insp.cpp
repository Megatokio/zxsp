// Copyright (c) 2002 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Z80Insp.h"
#include "Items/Ula/Ula.h"
#include "Machine.h"
#include "Qt/MyLineEdit.h"
#include "Templates/NVPtr.h"
#include "Uni/util.h"
#include "Z80/Z80.h"
#include "Z80/Z80opcodes.h"
#include "cpp/cppthreads.h"
#include <QGridLayout>
#include <QLabel>
#include <QTimer>


namespace gui
{

Z80Insp::Z80Insp(QWidget* window, MachineController* mc, volatile IsaObject* item) :
	Inspector(window, mc, item, "/Backgrounds/light-150-s.jpg")
{
	assert(object->isA(isa_Z80));
	background = background.scaled(200, 350);
	this->setFixedSize(background.size());

	memset(&value, 0, sizeof(value));
	value.clock = 0.0;

	QGridLayout* g = new QGridLayout(this);
	g->setContentsMargins(10, 10, 10, 10);
	g->setVerticalSpacing(4);
	g->setRowStretch(16, 100);
	g->setColumnStretch(0, 33);
	g->setColumnStretch(1, 66);

	clock = new_led("0.0 MHz");
	g->addWidget(new QLabel("Clock"), 0, 0, Qt::AlignRight);
	g->addWidget(clock, 0, 1);

	cc = new_led("00000");
	g->addWidget(new QLabel("Cpu cycle"), 1, 0, Qt::AlignRight);
	g->addWidget(cc, 1, 1);

	QHBoxLayout* h = new QHBoxLayout();
	h->setContentsMargins(0, 0, 0, 0);
	a = new_led("$00");
	h->addWidget(a);
	f = new_led("$00");
	h->addWidget(f);
	g->addWidget(new QLabel("AF"), 2, 0, Qt::AlignRight);
	g->addLayout(h, 2, 1);

	QHBoxLayout* h2 = new QHBoxLayout();
	h2->setContentsMargins(0, 0, 0, 0);
	a2 = new_led("$00");
	h2->addWidget(a2);
	f2 = new_led("$00");
	h2->addWidget(f2);
	g->addWidget(new QLabel("A'F'"), 3, 0, Qt::AlignRight);
	g->addLayout(h2, 3, 1);

	flags = new_led("--------");
	g->addWidget(new QLabel("Flags"), 4, 0, Qt::AlignRight);
	g->addWidget(flags, 4, 1);
	pc = new_led("$0000");
	g->addWidget(new QLabel("PC"), 5, 0, Qt::AlignRight);
	g->addWidget(pc, 5, 1);
	sp = new_led("$0000");
	g->addWidget(new QLabel("SP"), 6, 0, Qt::AlignRight);
	g->addWidget(sp, 6, 1);
	bc = new_led("$0000");
	g->addWidget(new QLabel("BC"), 7, 0, Qt::AlignRight);
	g->addWidget(bc, 7, 1);
	de = new_led("$0000");
	g->addWidget(new QLabel("DE"), 8, 0, Qt::AlignRight);
	g->addWidget(de, 8, 1);
	hl = new_led("$0000");
	g->addWidget(new QLabel("HL"), 9, 0, Qt::AlignRight);
	g->addWidget(hl, 9, 1);
	ix = new_led("$0000");
	g->addWidget(new QLabel("IX"), 10, 0, Qt::AlignRight);
	g->addWidget(ix, 10, 1);
	iy = new_led("$0000");
	g->addWidget(new QLabel("IY"), 11, 0, Qt::AlignRight);
	g->addWidget(iy, 11, 1);
	bc2 = new_led("$0000");
	g->addWidget(new QLabel("BC'"), 12, 0, Qt::AlignRight);
	g->addWidget(bc2, 12, 1);
	de2 = new_led("$0000");
	g->addWidget(new QLabel("DE'"), 13, 0, Qt::AlignRight);
	g->addWidget(de2, 13, 1);
	hl2 = new_led("$0000");
	g->addWidget(new QLabel("HL'"), 14, 0, Qt::AlignRight);
	g->addWidget(hl2, 14, 1);

	QHBoxLayout* h3 = new QHBoxLayout();
	h3->setContentsMargins(0, 0, 0, 0);
	im = new_led("0");
	h3->addWidget(im);
	i = new_led("$00");
	h3->addWidget(i);
	r = new_led("$00");
	h3->addWidget(r);
	g->addWidget(new QLabel("IM, I, R"), 15, 0, Qt::AlignRight);
	g->addLayout(h3, 15, 1);

	QHBoxLayout* h4 = new QHBoxLayout();
	h4->setContentsMargins(0, 0, 0, 0);
	irpt = new QCheckBox("Int", this);
	h4->addWidget(irpt, 0, Qt::AlignRight);
	h4->setStretch(0, 100);
	//	bool f = connect(irpt,SIGNAL(clicked(bool)),this,SLOT(slotIrptClicked(bool)));
	connect(irpt, &QCheckBox::clicked, this, &Z80Insp::set_interrupt);
	nmi = new QCheckBox("Nmi", this);
	h4->addWidget(nmi, 1);
	h4->setStretch(1, 0);
	//	f = f && connect(nmi,SIGNAL(clicked(bool)),this,SLOT(slotNmiClicked(bool)));
	connect(nmi, &QCheckBox::clicked, this, &Z80Insp::set_nmi);
	ie = new QCheckBox("IE", this);
	h4->addWidget(ie, 2);
	h4->setStretch(2, 0);
	//	f = f && connect(ie,SIGNAL(clicked(bool)),this,SLOT(slotIeClicked(bool)));
	connect(ie, &QCheckBox::clicked, this, &Z80Insp::set_interrupt_enable);
	g->addLayout(h4, 16, 0, 1, 2);

	timer->start(1000 / 10);
}


MyLineEdit* Z80Insp::new_led(cstr s)
{
	MyLineEdit* e = new MyLineEdit(s);
	connect(e, &MyLineEdit::returnPressed, this, [=] { return_pressed_in_lineedit(e); });
	return e;
}


/*	update displayed values
	called by QTimer started in this.c'tor
*/
void Z80Insp::updateWidgets()
{
	xxlogIn("Z80Insp::update");
	if (!object)
	{
		timer->stop();
		return;
	}
	volatile Z80* cpu = this->cpu();

#define SetRR(RR)                                 \
  if (regs.RR != value.RR)                        \
  {                                               \
	value.RR = regs.RR;                           \
	RR->setText(catstr("$", hexstr(regs.RR, 4))); \
  }
#define SetR(RR)                                  \
  if (regs.RR != value.RR)                        \
  {                                               \
	value.RR = regs.RR;                           \
	RR->setText(catstr("$", hexstr(regs.RR, 2))); \
  }

	const volatile Z80Regs& regs = cpu->getRegisters();
	SetRR(pc);
	SetRR(sp);
	SetRR(bc);
	SetRR(de);
	SetRR(hl);
	SetRR(bc2);
	SetRR(de2);
	SetRR(hl2);
	SetRR(ix);
	SetRR(iy);
	SetR(a);
	SetR(a2);
	SetR(f2);
	SetR(i);
	SetR(r);
	SetR(f);

#undef SetRR
#undef SetR

	if (regs.f != value.fstr)
	{
		value.fstr = regs.f;
		flags->setText(binstr(value.fstr, "--------", "SZ1H1VNC"));
	}

	if (regs.im != value.im)
	{
		value.im = regs.im;
		im->setText(tostr(value.im));
	}

	if (cpu->cpuCycle() != value.cc)
	{
		value.cc = cpu->cpuCycle();
		cc->setText(tostr(value.cc));
	}

	if (machine->cpu_clock != value.clock)
	{
		value.clock = machine->cpu_clock;
		clock->setText(MHzStr(value.clock));
	}

	ie->setChecked(regs.iff1);
	nmi->setChecked(cpu->nmiPending());
	irpt->setChecked(cpu->interruptPending());
}


void Z80Insp::return_pressed_in_lineedit(MyLineEdit* led)
{
	xlogIn("Z80Insp::returnPressed");

	// MyLineEdit* led = dynamic_cast<MyLineEdit*>(QObject::sender());
	cstr   text = led->text().toUtf8().data();
	uint32 n	= intValue(text);

	NVPtr<Z80> cpu(this->cpu());
	Z80Regs&   regs = cpu->getRegisters();

#define setR(REG)                            \
  if (led == REG)                            \
  {                                          \
	value.REG = regs.REG = n;                \
	led->setText(catstr("$", hexstr(n, 2))); \
  }                                          \
  else
#define setRR(REG)                           \
  if (led == REG)                            \
  {                                          \
	value.REG = regs.REG = n;                \
	led->setText(catstr("$", hexstr(n, 4))); \
  }                                          \
  else

	setR(a) setR(f) setR(a2) setR(f2) setR(i) setR(r) setRR(sp) setRR(pc) setRR(bc) setRR(de) setRR(hl) setRR(ix)
		setRR(iy) setRR(bc2) setRR(de2) setRR(hl2)

		// else:

		if (led == flags)
	{
		char* s			= upperstr(text);
		uint8 new_flags = 0;

		while (*s)
		{
			switch (*s++)
			{
			case 'V': new_flags |= V_FLAG; break; // parity / overflow
			case 'S': new_flags |= S_FLAG; break;
			case 'Z': new_flags |= Z_FLAG; break;
			case 'H': new_flags |= H_FLAG; break;
			case 'N': new_flags |= N_FLAG; break;
			case 'C': new_flags |= C_FLAG; break;
			}
		}

		value.fstr = regs.f = new_flags;
		led->setText(binstr(value.fstr, "--------", "SZ1H1VNC"));
	}

	else if (led == im)
	{
		if (n <= 2)
		{
			value.im = regs.im = n;
			led->setText(tostr(n));
		}
	}

	else if (led == cc)
	{
		Machine* m = cpu->getMachine();
		if (m->isSuspended())
		{
			uint32 cc_ffb = m->ula->cpuCycleOfFrameFlyback(); // TODO: ZX80++
			if (n > cc_ffb) n = cc_ffb;
			n = (n - cpu->cpuCycle() + cc_ffb) % cc_ffb;
			if (n) m->runCpuCycles(n);

			value.cc = cpu->cpuCycle();
			cstr s	 = tostr(value.cc);
			cc->setText(s);
			cc->QLineEdit::setText(s); // wg. Kbd Fokus wird der Text sonst nicht aktualisiert
		}
	}

	else if (led == clock)
	{
		Frequency v = mhzValue(text);
		if (v >= 1.0)
		{
			Machine* m = cpu->getMachine();
			m->setSpeedFromCpuClock(v);
			value.clock = m->cpu_clock;
			led->setText(MHzStr(value.clock));
		}
	}
}


/*	The user toggled the IE checkbox
	Note: signal 'clicked' is only sent on user action, click() and animateClick();
	not on other program actions like setDown(), setChecked() or toggle().
*/
void Z80Insp::set_interrupt_enable(bool checked) { NVPtr<Z80>(cpu())->getRegisters().iff = checked ? 0x0101 : 0x0000; }

/*	The user toggled the NMI checkbox
 */
void Z80Insp::set_nmi(bool checked)
{
	NVPtr<Z80> z80(cpu());
	if (checked) z80->triggerNmi();
	else z80->clearNmi();
}

/*	The user toggled the INT checkbox
 */
void Z80Insp::set_interrupt(bool checked)
{
	NVPtr<Z80> z80(cpu());
	if (checked) z80->raiseInterrupt();
	else z80->clearInterrupt();
}

} // namespace gui
