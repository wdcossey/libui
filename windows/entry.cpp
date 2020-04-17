// 8 april 2015
#include "uipriv_windows.hpp"
#include "keyboard.hpp"

struct uiEntry {
	uiWindowsControl c;
	HWND hwnd;
	void (*onChanged)(uiEntry *, void *);
	void *onChangedData;
	BOOL inhibitChanged;
	int (*onKeyEvent)(uiEntry *, uiAreaKeyEvent *);
	void *self;
	WNDPROC native_wndproc;
	HFONT hfont = NULL;
	int prefHeight = 14;       // from http://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing
	int prefWidth = 107;       // this is actually the shorter progress bar width, but Microsoft only indicates as wide as necessary
};


static LRESULT CALLBACK entryWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BOOL righthand = FALSE;
	uiAreaKeyEvent ke;
	ke.Key = 0;
	ke.ExtKey = 0;
	ke.Modifier = 0;
	ke.Modifiers = getModifiers();
	ke.Up = 0;

	int (*onKeyEvent)(uiEntry *, uiAreaKeyEvent *) =  (int (*)(uiEntry *, uiAreaKeyEvent *))GetProp(hwnd, L"ON_KEY_EVENT");
	uiEntry *self =  (uiEntry *)GetProp(hwnd, L"SELF");

	switch (uMsg)
	{
		case WM_GETDLGCODE:
			return DLGC_HASSETSEL | DLGC_WANTALLKEYS;
			break;
		case WM_KEYUP:
			ke.Up = 1;
		case WM_KEYDOWN:
			if (onKeyEvent) {
				fillKeyEvent(ke, wParam, lParam);
				if ((*onKeyEvent)(self, &ke)) {
					return TRUE;
				}
			}
			break;
	}

	WNDPROC native_wndproc = (WNDPROC)GetProp(hwnd, L"NATIVE_WNDPROC"); 
	return CallWindowProcW(native_wndproc, hwnd, uMsg, wParam, lParam);
}

static BOOL onWM_COMMAND(uiControl *c, HWND hwnd, WORD code, LRESULT *lResult)
{
	uiEntry *e = uiEntry(c);

	if (code != EN_CHANGE)
		return FALSE;
	if (e->inhibitChanged)
		return FALSE;
	(*(e->onChanged))(e, e->onChangedData);
	*lResult = 0;
	return TRUE;
}

static void uiEntryDestroy(uiControl *c)
{
	uiEntry *e = uiEntry(c);

	(WNDPROC)SetWindowLongPtrW(e->hwnd, GWLP_WNDPROC, (LONG_PTR)e->native_wndproc);
	RemoveProp(e->hwnd, L"NATIVE_WNDPROC");
	RemoveProp(e->hwnd, L"ON_KEY_EVENT");
	RemoveProp(e->hwnd, L"SELF");

	uiWindowsUnregisterWM_COMMANDHandler(e->hwnd);
	uiWindowsEnsureDestroyWindow(e->hwnd);
	if (e->hfont) {
		DeleteObject(e->hfont);
	}
	uiFreeControl(uiControl(e));
}

uiWindowsControlAllDefaultsExceptDestroy(uiEntry)

static void uiEntryMinimumSize(uiWindowsControl *c, int *width, int *height)
{
	uiEntry *e = uiEntry(c);
	uiWindowsSizing sizing;
	int x, y;

	x = e->prefWidth;
	y = e->prefHeight;
	uiWindowsGetSizingWithFont(e->hwnd, &sizing, e->hfont);
	uiWindowsSizingDlgUnitsToPixels(&sizing, &x, &y);
	*width = x;
	*height = y;
}

static void defaultOnChanged(uiEntry *e, void *data)
{
	// do nothing
}

static int defaultOnKeyEvent(uiEntry *e, uiAreaKeyEvent *event)
{
	// do nothing
	return FALSE;
}


void uiEntrySetFont(uiEntry *e, const char *name, int size, int weight, int italic)
{
	if (e->hfont) {
		DeleteObject(e->hfont);
	}
	e->hfont = CreateFontA(size, 0, 0, 0, weight, italic, FALSE, FALSE,
			ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, name);
	SendMessage(e->hwnd, WM_SETFONT, WPARAM (e->hfont), TRUE);
}

void uiEntryUnsetFocus(uiEntry *e)
{
	// stub to cover macos workaround
}

char *uiEntryText(uiEntry *e)
{
	return uiWindowsWindowText(e->hwnd);
}

void uiEntrySetText(uiEntry *e, const char *text)
{
	// doing this raises an EN_CHANGED
	e->inhibitChanged = TRUE;
	uiWindowsSetWindowText(e->hwnd, text);
	e->inhibitChanged = FALSE;
	// don't queue the control for resize; entry sizes are independent of their contents
}

void uiEntrySelectText(uiEntry *e, int start, int end)
{
	e->inhibitChanged = TRUE;
	uiWindowsSetlectWindowText(e->hwnd, start, end);
	e->inhibitChanged = FALSE;
}

void uiEntrySelectAllText(uiEntry *e)
{
	uiEntrySelectText(e, 0, -1);
}

void uiEntrySetMaxLength(uiEntry *e, int max)
{
	SendMessage(e->hwnd, EM_LIMITTEXT, WPARAM(max), TRUE);
}

void uiEntrySetPrefSize(uiEntry *e, int width, int height)
{
	e->prefWidth = width;
	e->prefHeight = height;
	uiWindowsControlMinimumSizeChanged(uiWindowsControl(e));
}

void uiEntryOnChanged(uiEntry *e, void (*f)(uiEntry *, void *), void *data)
{
	e->onChanged = f;
	e->onChangedData = data;
}

void uiEntryOnKeyEvent(uiEntry *e, int (*f)(uiEntry *, uiAreaKeyEvent *))
{
	e->onKeyEvent = f;
	SetProp(e->hwnd, L"ON_KEY_EVENT", (HGLOBAL)(f));
}

int uiEntryReadOnly(uiEntry *e)
{
	return (getStyle(e->hwnd) & ES_READONLY) != 0;
}

void uiEntryPasswordChar(uiEntry *e, char ch)
{
	if (SendMessage(e->hwnd, EM_SETPASSWORDCHAR, (WPARAM)ch, 0) == 0)
		logLastError(L"error setting password char");
}

void uiEntryCenterText(uiEntry *e, int center)
{
	DWORD style = GetWindowLongPtr(e->hwnd, GWL_STYLE);
	if (center == 0)
		style |= ~ES_CENTER;
	else
		style |= ES_CENTER;
	SetWindowLongPtrW(e->hwnd, GWL_STYLE, style);
}

void uiEntrySetReadOnly(uiEntry *e, int readonly)
{
	WPARAM ro;

	ro = (WPARAM) FALSE;
	if (readonly)
		ro = (WPARAM) TRUE;
	if (SendMessage(e->hwnd, EM_SETREADONLY, ro, 0) == 0)
		logLastError(L"error making uiEntry read-only");
}

static uiEntry *finishNewEntry(DWORD style)
{
	uiEntry *e;

	uiWindowsNewControl(uiEntry, e);
	e->prefHeight = 14;
	e->prefWidth = 107;

	e->hwnd = uiWindowsEnsureCreateControlHWND(WS_EX_CLIENTEDGE,
		L"edit", L"",
		style | ES_AUTOHSCROLL | ES_LEFT | ES_NOHIDESEL | WS_TABSTOP,
		hInstance, NULL,
		TRUE);

	uiWindowsRegisterWM_COMMANDHandler(e->hwnd, onWM_COMMAND, uiControl(e));

	e->native_wndproc = (WNDPROC)SetWindowLongPtrW(e->hwnd, GWLP_WNDPROC, (LONG_PTR)entryWndProc);
	SetProp(e->hwnd, L"NATIVE_WNDPROC", (HGLOBAL)e->native_wndproc);
	SetProp(e->hwnd, L"SELF", (HGLOBAL)e);

	uiEntryOnChanged(e, defaultOnChanged, NULL);
	uiEntryOnKeyEvent(e, defaultOnKeyEvent);

	return e;
}

uiEntry *uiNewEntry(void)
{
	return finishNewEntry(0);
}

uiEntry *uiNewPasswordEntry(void)
{
	return finishNewEntry(ES_PASSWORD);
}

uiEntry *uiNewSearchEntry(void)
{
	uiEntry *e;
	HRESULT hr;

	e = finishNewEntry(0);
	// TODO this is from ThemeExplorer; is it documented anywhere?
	// TODO SearchBoxEditComposited has no border
	hr = SetWindowTheme(e->hwnd, L"SearchBoxEdit", NULL);
	// TODO will hr be S_OK if themes are disabled?
	return e;
}
