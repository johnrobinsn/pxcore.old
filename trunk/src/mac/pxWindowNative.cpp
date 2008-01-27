// pxCore CopyRight 2007 John Robinson
// Portable Framebuffer and Windowing Library
// pwWindowNative.cpp

#include "pxWindow.h"
#include "pxWindowNative.h"

// pxWindow

pxError pxWindow::init(int left, int top, int width, int height)
{
	Rect r;
	r.top = top; r.left = left; r.bottom = top+height; r.right = left+width; 
	
	CreateNewWindow(kDocumentWindowClass,  
		kWindowCloseBoxAttribute | kWindowResizableAttribute | kWindowLiveResizeAttribute | 
		kWindowResizeTransitionAction | kWindowCollapseBoxAttribute | kWindowStandardHandlerAttribute,
		&r, &mWindowRef);

	if (mWindowRef)
	{
		EventTypeSpec		eventType;
		EventHandlerUPP		handlerUPP;
		EventHandlerRef handler;
			
		eventType.eventClass = kEventClassKeyboard;
		eventType.eventKind = kEventRawKeyDown;
		handlerUPP = NewEventHandlerUPP(doKeyDown);
		InstallApplicationEventHandler (handlerUPP, 1, &eventType, this, nil);

		eventType.eventKind = kEventRawKeyUp;
		handlerUPP = NewEventHandlerUPP(doKeyUp);
		InstallApplicationEventHandler (handlerUPP, 1, &eventType, this, nil);

		eventType.eventKind = kEventRawKeyRepeat;
		handlerUPP = NewEventHandlerUPP(doKeyDown);		// 'key repeat' is translated to 'key down'
		InstallApplicationEventHandler (handlerUPP, 1, &eventType, this, nil);	
		
		eventType.eventKind = kEventRawKeyModifiersChanged;
		handlerUPP = NewEventHandlerUPP(doKeyModifierChanged);
		InstallWindowEventHandler (mWindowRef, handlerUPP, 1, &eventType, this, &handler);	
		
		eventType.eventKind = kEventWindowDrawContent;
		eventType.eventClass = kEventClassWindow;
		handlerUPP = NewEventHandlerUPP(doWindowDrawContent);
		InstallWindowEventHandler (mWindowRef, handlerUPP, 1, &eventType, this, &handler);
		
		eventType.eventKind = kEventWindowBoundsChanging;
		eventType.eventClass = kEventClassWindow;

		handlerUPP = NewEventHandlerUPP(doWindowResizeComplete);
		InstallWindowEventHandler (mWindowRef, handlerUPP, 1, &eventType, this, &handler);
		
		eventType.eventKind = kEventWindowBoundsChanged;
		eventType.eventClass = kEventClassWindow;

		handlerUPP = NewEventHandlerUPP(doWindowResizeComplete);
		InstallWindowEventHandler (mWindowRef, handlerUPP, 1, &eventType, this, &handler);

		eventType.eventKind = kEventWindowClose;
		handlerUPP = NewEventHandlerUPP(doWindowCloseRequest);
		InstallWindowEventHandler (mWindowRef, handlerUPP, 1, &eventType, this, &handler);

		eventType.eventKind = kEventWindowClosed;
		eventType.eventClass = kEventClassWindow;

		handlerUPP = NewEventHandlerUPP(doWindowClosed);
		InstallWindowEventHandler (mWindowRef, handlerUPP, 1, &eventType, this, &handler);
		
		eventType.eventKind = kEventMouseDown;
		eventType.eventClass = kEventClassMouse;

		handlerUPP = NewEventHandlerUPP(doMouseDown);
		InstallWindowEventHandler (mWindowRef, handlerUPP, 1, &eventType, this, &handler);	
		
		eventType.eventKind = kEventMouseUp;
		eventType.eventClass = kEventClassMouse;

		handlerUPP = NewEventHandlerUPP(doMouseUp);
		InstallWindowEventHandler (mWindowRef, handlerUPP, 1, &eventType, this, &handler);	
		
		eventType.eventKind = kEventMouseMoved;
		eventType.eventClass = kEventClassMouse;

		handlerUPP = NewEventHandlerUPP(doMouseMove);
		InstallWindowEventHandler (mWindowRef, handlerUPP, 1, &eventType, this, &handler);	
		
		eventType.eventKind = kEventMouseDragged;
		eventType.eventClass = kEventClassMouse;

		handlerUPP = NewEventHandlerUPP(doMouseMove);
		InstallApplicationEventHandler (handlerUPP, 1, &eventType, this, nil);	
	
		{
			Rect r;
			GetWindowBounds(mWindowRef,kWindowContentRgn ,&r);
			onSize(r.right-r.left, r.bottom-r.top);
		}

		onCreate();
	}
	
	return mWindowRef?PX_OK:PX_FAIL;
}

pxError pxWindow::term()
{
	if (mWindowRef)
	{
		DisposeWindow(mWindowRef);
		mWindowRef = NULL;
	}
    return PX_OK;
}

void pxWindow::invalidateRect(pxRect* pxr)
{
	Rect r;
	if (!pxr)
	{
		GetWindowBounds(mWindowRef,kWindowContentRgn ,&r);
		r.right -= r.left;
		r.bottom -= r.top;
		r.left = 0;
		r.top = 0;
	}
	else
	{
		r.left = pxr->left();
		r.right = pxr->right();
		r.top = pxr->top();
		r.bottom = pxr->bottom();
	}
	InvalWindowRect(mWindowRef,&r);
}

bool pxWindow::visibility()
{
    return IsWindowVisible(mWindowRef);
}

void pxWindow::setVisibility(bool visible)
{
	if (visible) ShowWindow(mWindowRef);
	else HideWindow(mWindowRef);
}

pxError pxWindow::setAnimationFPS(long fps)
{
	if (theTimer)
	{
		RemoveEventLoopTimer(theTimer);
	}

	if (fps > 0)
	{
		EventLoopRef		mainLoop;
		EventLoopTimerUPP	timerUPP;

		mainLoop = GetMainEventLoop();
		timerUPP = NewEventLoopTimerUPP (doPeriodicTask);
		InstallEventLoopTimer (mainLoop, 0, (1000/fps) * kEventDurationMillisecond, timerUPP, this, &theTimer);
	}
    return PX_OK;
}

void pxWindow::setTitle(char* title)
{
	SetWindowTitleWithCFString(mWindowRef,CFStringCreateWithCString(nil,title,kCFStringEncodingASCII));
}

// pxWindowNative

pascal OSStatus pxWindowNative::doKeyDown (EventHandlerCallRef nextHandler, EventRef theEvent, void* userData)
{
	char key_code;
	UInt32 modifier;
	
	GetEventParameter (theEvent, kEventParamKeyMacCharCodes, typeChar, NULL, sizeof(char), NULL, &key_code);
	GetEventParameter (theEvent, kEventParamKeyModifiers, typeUInt32, NULL, sizeof(UInt32), NULL, &modifier);
	
	UInt32 kc;
	GetEventParameter (theEvent, kEventParamKeyCode, typeUInt32, NULL, sizeof(UInt32), NULL, &kc);
//	printf("VK DN: %lx\n", kc);

	if (kc == 0x24) kc = 0x4c;
	
	pxWindowNative* w = (pxWindowNative*)userData;

	unsigned long flags = 0;
	
	if (modifier & shiftKey) flags |= PX_MOD_SHIFT;
	if (modifier & optionKey) flags |= PX_MOD_ALT;
	if (modifier & controlKey) flags |= PX_MOD_CONTROL;

	w->onKeyDown(kc, flags);

	return CallNextEventHandler (nextHandler, theEvent);
}


//------------------------------------------------------------------------
pascal OSStatus pxWindowNative::doKeyUp (EventHandlerCallRef nextHandler, EventRef theEvent, void* userData)
{
	char key_code;
	char char_code;
	UInt32 modifier;
	
	GetEventParameter (theEvent, kEventParamKeyMacCharCodes, typeChar, NULL, sizeof(char), NULL, &key_code);
	GetEventParameter (theEvent, kEventParamKeyModifiers, typeUInt32, NULL, sizeof(UInt32), NULL, &modifier);

	UInt32 kc;
	GetEventParameter (theEvent, kEventParamKeyCode, typeUInt32, NULL, sizeof(UInt32), NULL, &kc);
//	printf("VK UP: %lx\n", kc);

	pxWindowNative* w = (pxWindowNative*)userData;

	if (kc == 0x24) kc = 0x4c;

	unsigned long flags = 0;
	
	if (modifier & shiftKey) flags |= PX_MOD_SHIFT;
	if (modifier & optionKey) flags |= PX_MOD_ALT;
	if (modifier & controlKey) flags |= PX_MOD_CONTROL;

	w->onKeyUp(kc, flags);

	return CallNextEventHandler (nextHandler, theEvent);
}

pascal OSStatus pxWindowNative::doKeyModifierChanged (EventHandlerCallRef nextHandler, EventRef theEvent, void* userData)
{
	UInt32 modifier;
	
	GetEventParameter (theEvent, kEventParamKeyModifiers, typeUInt32, NULL, sizeof(UInt32), NULL, &modifier);

	pxWindowNative* w = (pxWindowNative*)userData;

	if (!(w->mLastModifierState & shiftKey) && (modifier & shiftKey)) w->onKeyDown(PX_KEY_SHIFT, 0); 
	else if ((w->mLastModifierState & shiftKey) && !(modifier & shiftKey)) w->onKeyUp(PX_KEY_SHIFT, 0);
	
	if (!(w->mLastModifierState & optionKey) && (modifier & optionKey)) w->onKeyDown(PX_KEY_ALT, 0); 
	else if ((w->mLastModifierState & optionKey) && !(modifier & optionKey)) w->onKeyUp(PX_KEY_ALT, 0);
	
	if (!(w->mLastModifierState & controlKey) && (modifier & controlKey)) w->onKeyDown(PX_KEY_CONTROL, 0); 
	else if ((w->mLastModifierState & controlKey) && !(modifier & controlKey)) w->onKeyUp(PX_KEY_CONTROL, 0);
	
	if (!(w->mLastModifierState & alphaLock) && (modifier & alphaLock)) w->onKeyDown(PX_KEY_CAPSLOCK, 0); 
	else if ((w->mLastModifierState & alphaLock) && !(modifier & alphaLock)) w->onKeyUp(PX_KEY_CAPSLOCK, 0);
	
	w->mLastModifierState = modifier;

	return CallNextEventHandler (nextHandler, theEvent);
}

pascal OSStatus pxWindowNative::doWindowDrawContent (EventHandlerCallRef nextHandler, EventRef theEvent, void* userData)
{
	pxWindowNative* w = (pxWindowNative*)userData;
	SetPort(GetWindowPort(w->mWindowRef));
	w->onDraw(GetPortPixMap(GetWindowPort(w->mWindowRef)));

	return CallNextEventHandler (nextHandler, theEvent);
}	

pascal OSStatus pxWindowNative::doWindowResizeComplete(EventHandlerCallRef nextHandler, EventRef theEvent, void* userData)
{
	pxWindowNative* w = (pxWindowNative*)userData;
	
    Rect rect;
    GetEventParameter(theEvent, kEventParamCurrentBounds,
        typeQDRectangle, NULL, sizeof(Rect), NULL, &rect);
		
	w->onSize(rect.right-rect.left, rect.bottom-rect.top);

	Rect r;
	GetWindowBounds(w->mWindowRef,kWindowContentRgn ,&r);
	r.right -= r.left;
	r.bottom -= r.top;
	r.left = r.top = 0;
	InvalWindowRect(w->mWindowRef,&r);

	return CallNextEventHandler (nextHandler, theEvent);
}	

pascal OSStatus pxWindowNative::doWindowCloseRequest(EventHandlerCallRef nextHandler, EventRef theEvent, void* userData)
{
	pxWindowNative* w = (pxWindowNative*)userData;
	printf("here\n");
	w->onCloseRequest();
	return noErr;  // Don't use default handler
}

pascal void pxWindowNative::doPeriodicTask (EventLoopTimerRef theTimer, void* userData)
{
	pxWindowNative* w = (pxWindowNative*)userData;
	w->onAnimationTimer();
}

pascal OSStatus pxWindowNative::doWindowClosed(EventHandlerCallRef nextHandler, EventRef theEvent, void* userData)
{
	pxWindowNative* w = (pxWindowNative*)userData;
	w->onClose();

	return CallNextEventHandler (nextHandler, theEvent);
}	

pascal OSStatus pxWindowNative::doMouseDown(EventHandlerCallRef nextHandler, EventRef theEvent, void* userData)
{
	pxWindowNative* w = (pxWindowNative*)userData;

	Point loc;
	UInt16 button;
	UInt32 modifier;
	
	GetEventParameter (theEvent, kEventParamMouseLocation, typeQDPoint, NULL, sizeof(Point), NULL, &loc);
	SetPort(GetWindowPort(w->mWindowRef));
	GlobalToLocal (&loc);
	GetEventParameter(theEvent, kEventParamMouseButton, typeMouseButton, NULL, sizeof(button), NULL, &button);
	GetEventParameter(theEvent, kEventParamKeyModifiers, typeUInt32, NULL, sizeof(UInt32), NULL, &modifier);	

	unsigned long flags = 0;

	if (button == kEventMouseButtonPrimary) flags |= PX_LEFTBUTTON;
	else if (button == kEventMouseButtonSecondary) flags |= PX_RIGHTBUTTON;
	else if (button == kEventMouseButtonTertiary) flags |= PX_MIDDLEBUTTON;
	
	if (modifier & shiftKey) flags |= PX_MOD_SHIFT;
	if (modifier & optionKey) flags |= PX_MOD_ALT;
	if (modifier & controlKey) flags |= PX_MOD_CONTROL;

	w->onMouseDown(loc.h, loc.v, flags);

	return CallNextEventHandler (nextHandler, theEvent);
}	

pascal OSStatus pxWindowNative::doMouseUp(EventHandlerCallRef nextHandler, EventRef theEvent, void* userData)
{
	pxWindowNative* w = (pxWindowNative*)userData;
	
	Point loc;
	UInt16 button;
	UInt32 modifier;
	
	GetEventParameter (theEvent, kEventParamMouseLocation, typeQDPoint, NULL, sizeof(Point), NULL, &loc);
	SetPort(GetWindowPort(w->mWindowRef));
	GlobalToLocal(&loc);
	GetEventParameter(theEvent, kEventParamMouseButton, typeMouseButton, NULL, sizeof(button), NULL, &button);
	GetEventParameter(theEvent, kEventParamKeyModifiers, typeUInt32, NULL, sizeof(UInt32), NULL, &modifier);	
	
	unsigned long flags = 0;

	if (button == kEventMouseButtonPrimary) flags |= PX_LEFTBUTTON;
	else if (button == kEventMouseButtonSecondary) flags |= PX_RIGHTBUTTON;
	else if (button == kEventMouseButtonTertiary) flags |= PX_MIDDLEBUTTON;
	
	if (modifier & shiftKey) flags |= PX_MOD_SHIFT;
	if (modifier & optionKey) flags |= PX_MOD_ALT;
	if (modifier & controlKey) flags |= PX_MOD_CONTROL;

	w->onMouseUp(loc.h, loc.v, flags);

	return CallNextEventHandler (nextHandler, theEvent);
}	

pascal OSStatus pxWindowNative::doMouseMove(EventHandlerCallRef nextHandler, EventRef theEvent, void* userData)
{
	pxWindowNative* w = (pxWindowNative*)userData;
	
	Point loc;
	
	GetEventParameter (theEvent, kEventParamMouseLocation, typeQDPoint, NULL, sizeof(Point), NULL, &loc);
	SetPort(GetWindowPort(w->mWindowRef));
	GlobalToLocal (&loc);
	
	
	w->onMouseMove(loc.h, loc.v);

	return CallNextEventHandler (nextHandler, theEvent);
}	
