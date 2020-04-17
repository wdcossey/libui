// 11 april 2015
#include "uipriv_windows.hpp"
#include "../common/general.h"

struct uiLabel {
	uiWindowsControl c;
	HFONT hfont = NULL;
	HWND hwnd;
	int minHeight = -1;
	int minWidth = -1;
};

static void uiLabelDestroy(uiControl *c)
{
	uiLabel *l = uiLabel(c);
	uiWindowsEnsureDestroyWindow(l->hwnd);
	if (l->hfont) {
		DeleteObject(l->hfont);
	}
	uiFreeControl(uiControl(l));
}

uiWindowsControlAllDefaultsExceptDestroy(uiLabel)

// via http://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing
#define labelHeight 8

static void uiLabelMinimumSize(uiWindowsControl *c, int *width, int *height)
{
	uiLabel *l = uiLabel(c);
	uiWindowsSizing sizing;
	int y;

	*width = max(uiWindowsWindowTextWidth(l->hwnd), l->minWidth);
	y = labelHeight;
	uiWindowsGetSizingWithFont(l->hwnd, &sizing, l->hfont);
	uiWindowsSizingDlgUnitsToPixels(&sizing, NULL, &y);
	*height = max(y, l->minHeight);
}

char *uiLabelText(uiLabel *l)
{
	return uiWindowsWindowText(l->hwnd);
}

void uiLabelSetText(uiLabel *l, const char *text)
{
	uiWindowsSetWindowText(l->hwnd, text);
	// changing the text might necessitate a change in the label's size
	uiWindowsControlMinimumSizeChanged(uiWindowsControl(l));
}

void uiLabelSetFont(uiLabel *l, const char *name, int size, int weight, int italic)
{
	if (l->hfont) {
		DeleteObject(l->hfont);
	}
	l->hfont = CreateFontA(size, 0, 0, 0, weight, italic, FALSE, FALSE,
			ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, name);
	SendMessage(l->hwnd, WM_SETFONT, WPARAM (l->hfont), TRUE);
}

void uiLabelSetMinSize(uiLabel *l, int width, int height)
{
	l->minHeight = height;
	l->minWidth = width;
	uiWindowsControlMinimumSizeChanged(uiWindowsControl(l));
}

uiLabel *uiNewLabel(const char *text)
{
	uiLabel *l;
	WCHAR *wtext;

	uiWindowsNewControl(uiLabel, l);
	l->minHeight = -1;
	l->minWidth = -1;

	wtext = toUTF16(text);
	l->hwnd = uiWindowsEnsureCreateControlHWND(0,
		L"static", wtext,
		// SS_LEFTNOWORDWRAP clips text past the end; SS_NOPREFIX avoids accelerator translation
		// controls are vertically aligned to the top by default (thanks Xeek in irc.freenode.net/#winapi)
		SS_LEFTNOWORDWRAP | SS_NOPREFIX,
		hInstance, NULL,
		TRUE);
	uiprivFree(wtext);

	return l;
}

uiLabel *uiNewCenteredLabel(const char *text)
{
	uiLabel *l;
	WCHAR *wtext;

	uiWindowsNewControl(uiLabel, l);
	l->minHeight = -1;
	l->minWidth = -1;

	wtext = toUTF16(text);
	l->hwnd = uiWindowsEnsureCreateControlHWND(0,
		L"static", wtext,
		// SS_LEFTNOWORDWRAP clips text past the end; SS_NOPREFIX avoids accelerator translation
		// controls are vertically aligned to the top by default (thanks Xeek in irc.freenode.net/#winapi)
		SS_CENTER | SS_NOPREFIX,
		hInstance, NULL,
		TRUE);
	uiprivFree(wtext);

	return l;
}
