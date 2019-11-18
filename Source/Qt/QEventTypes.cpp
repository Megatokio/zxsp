/*	Copyright  (c)	Günter Woigk 2009 - 2019
					mailto:kio@little-bat.de

	This file is free software.

	Permission to use, copy, modify, distribute, and sell this software
	and its documentation for any purpose is hereby granted without fee,
	provided that the above copyright notice appears in all copies and
	that both that copyright notice, this permission notice and the
	following disclaimer appear in supporting documentation.

	THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT ANY WARRANTY,
	NOT EVEN THE IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS FOR
	A PARTICULAR PURPOSE, AND IN NO EVENT SHALL THE COPYRIGHT HOLDER
	BE LIABLE FOR ANY DAMAGES ARISING FROM THE USE OF THIS SOFTWARE,
	TO THE EXTENT PERMITTED BY APPLICABLE LAW.
*/

#include <QEvent>
#include "kio/kio.h"
#include "globals.h"


cstr QEventTypeStr(int /*QEvent::Type*/ n)
{
	switch(n)
	{
	case 0:	 return"0:None: invalid event";
	case 1:  return"1:Timer: timer event";
	case 2:  return"2:MouseButtonPress: mouse button pressed";
	case 3:  return"3:MouseButtonRelease: mouse button released";
	case 4:  return"4:MouseButtonDblClick: mouse button double click";
	case 5:  return"5:MouseMove: mouse move";
	case 6:  return"6:KeyPress: key pressed";
	case 7:  return"7:KeyRelease: key released";
	case 8:  return"8:FocusIn: keyboard focus received";
	case 9:  return"9:FocusOut: keyboard focus lost";
	case 10: return"10:Enter: mouse enters widget";
	case 11: return"11:Leave: mouse leaves widget";
	case 12: return"12:Paint: paint widget";
	case 13: return"13:Move: move widget";
	case 14: return"14:Resize: resize widget";
	case 15: return"15:Create: after widget creation";
	case 16: return"16:Destroy: during widget destruction";
	case 17: return"17:Show: widget is shown";
	case 18: return"18:Hide: widget is hidden";
	case 19: return"19:Close: request to close widget";
	case 20: return"20:Quit: request to quit application";
	case 21: return"21:ParentChange: widget has been reparented";
	case 22: return"22:ThreadChange: object has changed threads";
	case 23: return"23:FocusAboutToChange: keyboard focus is about to be lost";
	case 24: return"24:WindowActivate: window was activated";
	case 25: return"25:WindowDeactivate: window was deactivated";
	case 26: return"26:ShowToParent: widget is shown to parent";
	case 27: return"27:HideToParent: widget is hidden to parent";
//	case 28:
//	case 29:
	case 30: return"30:Accel: accelerator event (QT3)";
	case 31: return"31:Wheel: wheel event";
	case 32: return"32:AccelAvailable: accelerator available event (QT3)";
	case 33: return"33:WindowTitleChange: window title changed";
	case 34: return"34:WindowIconChange: icon changed";
	case 35: return"35:ApplicationWindowIconChange: application icon changed";
	case 36: return"36:ApplicationFontChange: application font changed";
	case 37: return"37:ApplicationLayoutDirectionChange: application layout direction changed";
	case 38: return"38:ApplicationPaletteChange: application palette changed";
	case 39: return"39:PaletteChange: widget palette changed";
	case 40: return"40:Clipboard: internal clipboard event";
//	case 41:
	case 42: return"42:Speech: reserved for speech input";
	case 43: return"43:MetaCall: meta call event";
//	case 44:
//	...
//	case 49:
	case 50: return"50:SockAct: socket activation";
	case 51: return"51:ShortcutOverride: shortcut override request";
	case 52: return"52:DeferredDelete: deferred delete event";
//	case 53:
//	...
//	case 59:
	case 60: return"60:DragEnter: drag moves into widget";
	case 61: return"61:DragMove: drag moves in widget";
	case 62: return"62:DragLeave: drag leaves or is cancelled";
	case 63: return"63:Drop: actual drop";
	case 64: return"64:DragResponse: drag accepted/rejected";
//	case 65:
//	case 66:
	case 67: return"67:ChildInsertedRequest: send ChildInserted compatibility events to receiver (QT3)";
	case 68: return"68:ChildAdded: new child widget";
	case 69: return"69:ChildPolished: polished child widget";
	case 70: return"70:ChildInserted: compatibility child inserted (QT3)";
	case 72: return"71:LayoutHint: compatibility relayout request (QT3)";
	case 71: return"72:ChildRemoved: deleted child widget";
	case 73: return"73:ShowWindowRequest: widget's window should be mapped";
	case 74: return"74:PolishRequest: widget should be polished";
	case 75: return"75:Polish: widget is polished";
	case 76: return"76:LayoutRequest: widget should be relayouted";
	case 77: return"77:UpdateRequest: widget should be repainted";
	case 78: return"78:UpdateLater: request update() later";
	case 79: return"79:EmbeddingControl: ActiveX embedding";
	case 80: return"80:ActivateControl: ActiveX activation";
	case 81: return"81:DeactivateControl: ActiveX deactivation";
	case 82: return"82:ContextMenu: context popup menu";
	case 83: return"83:InputMethod: input method";
//	case 84:
//	case 85:
	case 86: return"86:AccessibilityPrepare: accessibility information is requested";
	case 87: return"87:TabletMove: Wacom tablet event";
	case 88: return"88:LocaleChange: the system locale changed";
	case 89: return"89:LanguageChange: the application language changed";
	case 90: return"90:LayoutDirectionChange: the layout direction changed";
	case 91: return"91:Style: internal style event";
	case 92: return"92:TabletPress: tablet press";
	case 93: return"93:TabletRelease: tablet release";
	case 94: return"94:OkRequest: CE (Ok) button pressed";
	case 95: return"95:HelpRequest: CE (?)  button pressed";
	case 96: return"96:IconDrag: proxy icon dragged";
	case 97: return"97:FontChange: font has changed";
	case 98: return"98:EnabledChange: enabled state has changed";
	case 99: return"99:ActivationChange: window activation has changed";
	case 100:return"100:StyleChange: style has changed";
	case 101:return"101:IconTextChange: icon text has changed";
	case 102:return"102:ModifiedChange: modified state has changed";
	case 103:return"103:WindowBlocked: window is about to be blocked modally";
	case 104:return"104:WindowUnblocked: windows modal blocking has ended";
	case 105:return"105:WindowStateChange";
//	case 106:
//	case 107:
//	case 108:
	case 109:return"109:MouseTrackingChange = 109: mouse tracking state has changed";
	case 110:return"110:ToolTip";
	case 111:return"111:WhatsThis";
	case 112:return"112:StatusTip";
	case 113:return"113:ActionChanged";
	case 114:return"114:ActionAdded";
	case 115:return"115:ActionRemoved";
	case 116:return"116:FileOpen: file open request";
	case 117:return"117:Shortcut: shortcut triggered";
	case 118:return"118:WhatsThisClicked";
	case 119:return"119:AccessibilityHelp: accessibility help text request";
	case 120:return"120:ToolBarChange: toolbar visibility toggled";
	case 121:return"121:ApplicationActivate: application has been changed to active";
	case 122:return"122:ApplicationDeactivate: application has been changed to inactive";
	case 123:return"123:QueryWhatsThis: query what's this widget help";
	case 124:return"124:EnterWhatsThisMode";
	case 125:return"125:LeaveWhatsThisMode";
	case 126:return"126:ZOrderChange: child widget has had its z-order changed";
	case 127:return"127:HoverEnter: mouse cursor enters a hover widget";
	case 128:return"128:HoverLeave: mouse cursor leaves a hover widget";
	case 129:return"129:HoverMove: mouse cursor move inside a hover widget";
	case 130:return"130:AccessibilityDescription: accessibility description text request";
	case 131:return"131:ParentAboutToChange: sent just before the parent change is done";
	case 132:return"132:WinEventAct: win event activation";
//	case 133
//	...
//	case 149:
	case 150:return"150:EnterEditFocus: enter edit mode in QT_KEYPAD_NAVIGATION";
	case 151:return"151:LeaveEditFocus: leave edit mode in QT_KEYPAD_NAVIGATION";
	case 152:return"152:AcceptDropsChange";
	case 153:return"153:MenubarUpdated: Support event for Q3MainWindow (Qt3)";
	case 154:return"154:ZeroTimerEvent: Used for Windows Zero timer events";
	case 155:return"155:GraphicsSceneMouseMove";
	case 156:return"156:GraphicsSceneMousePress";
	case 157:return"157:GraphicsSceneMouseRelease";
	case 158:return"158:GraphicsSceneMouseDoubleClick";
	case 159:return"159:GraphicsSceneContextMenu";
	case 160:return"160:GraphicsSceneHoverEnter";
	case 161:return"161:GraphicsSceneHoverMove";
	case 162:return"162:GraphicsSceneHoverLeave";
	case 163:return"163:GraphicsSceneHelp";
	case 164:return"164:GraphicsSceneDragEnter";
	case 165:return"165:GraphicsSceneDragMove";
	case 166:return"166:GraphicsSceneDragLeave";
	case 167:return"167:GraphicsSceneDrop";
	case 168:return"168:GraphicsSceneWheel";
	case 169:return"169:KeyboardLayoutChange";
	case 170:return"170:DynamicPropertyChange: A dynamic property was changed through setProperty/property";
	case 171:return"171:TabletEnterProximity";
	case 172:return"172:TabletLeaveProximity";
	case 173:return"173:NonClientAreaMouseMove";
	case 174:return"174:NonClientAreaMouseButtonPress";
	case 175:return"175:NonClientAreaMouseButtonRelease";
	case 176:return"176:NonClientAreaMouseButtonDblClick";
	case 177:return"177:MacSizeChange: when the Qt::WA_Mac{Normal,Small,Mini}Size changes";
	case 178:return"178:ContentsRectChange: sent by QWidget::setContentsMargins (internal)";
	case 179:return"179:MacGLWindowChange: the window of the GLWidget has changed (internal)";
	case 180:return"180:FutureCallOut";
	case 181:return"181:GraphicsSceneResize";
	case 182:return"182:GraphicsSceneMove";
	case 183:return"183:CursorChange";
	case 184:return"184:ToolTipChange";
	case 185:return"185:NetworkReplyUpdated: QNetworkReply (internal)";
	case 186:return"186:GrabMouse";
	case 187:return"187:UngrabMouse";
	case 188:return"188:GrabKeyboard";
	case 189:return"189:UngrabKeyboard";
	case 190:return"190:CocoaRequestModal: requesting an application modal Cocoa Window (internal)";
	case 191:return"191:MacGLClearDrawable: Cocoa, the window has changed, so we must clear (internal)";
	case 192:return"192:StateMachineSignal";
	case 193:return"193:StateMachineWrapped";
	case 194:return"194:TouchBegin";
	case 195:return"195:TouchUpdate";
	case 196:return"196:TouchEnd";
	case 197:return"197:NativeGesture";
	case 198:return"198:Gesture";
	case 199:return"199:RequestSoftwareInputPanel";
	case 200:return"200:CloseSoftwareInputPanel";
	case 201:return"201:";
	case 202:return"202:GestureOverride";
	case 203:return"203:WinIdChange";
	case 204:return"204:ScrollPrepare";
	case 205:return"205:Scroll";
	case 206:return"206:Expose";
	case 207:return"207:InputMethodQuery";
	case 208:return"208:OrientationChange";
	case 209:return"209:TouchCancel";
	case 210:return"210:ThemeChange";
	case 211:return"211:SockClose: Socket closed";
	case 212:return"212:PlatformPanel";
	case 213:return"213:StyleAnimationUpdate: style animation target should be updated";
	case 214:return"214:ApplicationStateChange";

	case 512:return"512:reserved for Qt Jambi's MetaCall event";
	case 513:return"513:reserved for Qt Jambi's DeleteOnMainThread event";

	default: return catstr(tostr(n)," = ?");
	}
}





#if 0
//	case ApplicationActivated = ApplicationActivate, // deprecated
//	case ApplicationDeactivated = ApplicationDeactivate, // deprecated
//	case Reparent = ParentChange (QT3)
//	case CaptionChange = WindowTitleChange (QT3)
//	case IconChange = WindowIconChange (QT3)
//	case AccelOverride=ShortcutOverride: accelerator override event (QT3)
//	last event id used = 132
//	User = 1000,                            // first user event id
//	MaxUser = 65535                         // last user event id
#endif


