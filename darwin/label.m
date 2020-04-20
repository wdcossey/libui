// 14 august 2015
#import "uipriv_darwin.h"


@interface libui_labelNSTextField : NSTextField
@property int pref_width;
@property int pref_height;
- (void)setPrefSize:(int)w :(int)h;
@end

@implementation libui_labelNSTextField

- (void)setPrefSize:(int)w :(int)h
{
	self.pref_width = w;
	self.pref_height = h;
}

- (NSSize)intrinsicContentSize
{
	NSSize s;

	s = [super intrinsicContentSize];
	if (self.pref_width > 0) s.width = self.pref_width;
	if (self.pref_height > 0) s.height = self.pref_height;
	return s;
}

@end

struct uiLabel {
	uiDarwinControl c;
	NSTextField *textfield;
};

uiDarwinControlAllDefaults(uiLabel, textfield)

char *uiLabelText(uiLabel *l)
{
	return uiDarwinNSStringToText([l->textfield stringValue]);
}

void uiLabelSetText(uiLabel *l, const char *text)
{
	[l->textfield setStringValue:uiprivToNSString(text)];
}

void uiLabelSetFont(uiLabel *l, const char *name, int size, int weight, int italic)
{
	NSFontTraitMask traits = 0;
	if (italic > 0)
		traits = NSFontItalicTrait;
	NSFont* font = [[NSFontManager sharedFontManager]
		fontWithFamily:uiprivToNSString(name) traits:traits weight:systemTextWeigth((uiTextWeight)weight) size:size];
	[l->textfield setFont:font];
}

void uiLabelSetMinSize(uiLabel *l, int width, int height)
{
	[(libui_labelNSTextField *)l->textfield setPrefSize:width:height];
}

NSTextField *uiprivNewLabel(NSString *str)
{
	NSTextField *tf;

	tf = [[libui_labelNSTextField alloc] initWithFrame:NSZeroRect];
	[tf setStringValue:str];
	[tf setEditable:NO];
	[tf setSelectable:NO];
	[tf setDrawsBackground:NO];
	uiprivFinishNewTextField(tf, NO);
	return tf;
}

uiLabel *uiNewLabel(const char *text)
{
	uiLabel *l;

	uiDarwinNewControl(uiLabel, l);

	l->textfield = uiprivNewLabel(uiprivToNSString(text));

	return l;
}


uiLabel *uiNewCenteredLabel(const char *text)
{
	uiLabel *l;

	uiDarwinNewControl(uiLabel, l);

	l->textfield = uiprivNewLabel(uiprivToNSString(text));
	[l->textfield setAlignment:NSCenterTextAlignment];

	return l;
}
