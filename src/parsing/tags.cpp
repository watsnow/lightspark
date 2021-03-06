/**************************************************************************
    Lightspark, a free flash player implementation

    Copyright (C) 2009-2011  Alessandro Pignotti (a.pignotti@sssup.it)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

#include <libxml++/libxml++.h>
#include <libxml++/parsers/textreader.h>

#include <vector>
#include <list>
#include <algorithm>
#include <sstream>
#include "scripting/abc.h"
#include "tags.h"
#include "backends/geometry.h"
#include "backends/security.h"
#include "swftypes.h"
#include "logger.h"
#include "compat.h"
#include "streams.h"
#include "scripting/flash/display/BitmapData.h"

#undef RGB

using namespace std;
using namespace lightspark;

Tag* TagFactory::readTag(RootMovieClip* root)
{
	RECORDHEADER h;

	//Catch eofs
	try
	{
		f >> h;
	}
	catch (ifstream::failure e) {
		if(!f.eof()) //Only handle eof
			throw e;
		f.clear();
		LOG(LOG_INFO,"Simulating EndTag at EOF @ " << f.tellg());
		return new EndTag(h,f);
	}

	unsigned int expectedLen=h.getLength();
	unsigned int start=f.tellg();
	Tag* ret=NULL;
	LOG(LOG_TRACE,_("Reading tag type: ") << h.getTagType() << _(" at byte ") << start << _(" with length ") << expectedLen << _(" bytes"));
	switch(h.getTagType())
	{
		case 0:
			ret=new EndTag(h,f);
			break;
		case 1:
			ret=new ShowFrameTag(h,f);
			break;
		case 2:
			ret=new DefineShapeTag(h,f,root);
			break;
	//	case 4:
	//		ret=new PlaceObjectTag(h,f);
		case 6:
			ret=new DefineBitsTag(h,f,root);
			break;
		case 9:
			ret=new SetBackgroundColorTag(h,f);
			break;
		case 10:
			ret=new DefineFontTag(h,f,root);
			break;
		case 11:
			ret=new DefineTextTag(h,f,root);
			break;
		case 13:
			ret=new DefineFontInfoTag(h,f);
			break;
		case 14:
			ret=new DefineSoundTag(h,f,root);
			break;
		case 15:
			ret=new StartSoundTag(h,f);
			break;
		case 18:
			ret=new SoundStreamHeadTag(h,f);
			break;
		case 19:
			ret=new SoundStreamBlockTag(h,f);
			break;
		case 20:
			ret=new DefineBitsLosslessTag(h,f,1,root);
			break;
		case 21:
			ret=new DefineBitsJPEG2Tag(h,f,root);
			break;
		case 22:
			ret=new DefineShape2Tag(h,f,root);
			break;
		case 24:
			ret=new ProtectTag(h,f);
			break;
		case 26:
			ret=new PlaceObject2Tag(h,f,root);
			break;
		case 28:
			ret=new RemoveObject2Tag(h,f);
			break;
		case 32:
			ret=new DefineShape3Tag(h,f,root);
			break;
		case 33:
			ret=new DefineText2Tag(h,f,root);
			break;
		case 34:
			ret=new DefineButton2Tag(h,f,root);
			break;
		case 35:
			ret=new DefineBitsJPEG3Tag(h,f,root);
			break;
		case 36:
			ret=new DefineBitsLosslessTag(h,f,2,root);
			break;
		case 37:
			ret=new DefineEditTextTag(h,f,root);
			break;
		case 39:
			ret=new DefineSpriteTag(h,f,root);
			break;
		case 41:
			ret=new ProductInfoTag(h,f);
			break;
		case 43:
			ret=new FrameLabelTag(h,f);
			break;
		case 45:
			ret=new SoundStreamHead2Tag(h,f);
			break;
		case 46:
			ret=new DefineMorphShapeTag(h,f,root);
			break;
		case 48:
			ret=new DefineFont2Tag(h,f,root);
			break;
		case 58:
			ret=new EnableDebuggerTag(h,f);
			break;
		case 60:
			ret=new DefineVideoStreamTag(h,f,root);
			break;
		case 63:
			ret=new DebugIDTag(h,f);
			break;
		case 64:
			ret=new EnableDebugger2Tag(h,f);
			break;
		case 65:
			ret=new ScriptLimitsTag(h,f);
			break;
		case 69:
			//FileAttributes tag is mandatory on version>=8 and must be the first tag
			if(!firstTag)
				LOG(LOG_ERROR,_("FileAttributes tag not in the beginning"));
			ret=new FileAttributesTag(h,f);
			break;
		case 70:
			ret=new PlaceObject3Tag(h,f,root);
			break;
		case 72:
			ret=new DoABCTag(h,f);
			break;
		case 73:
			ret=new DefineFontAlignZonesTag(h,f);
			break;
		case 74:
			ret=new CSMTextSettingsTag(h,f);
			break;
		case 75:
			ret=new DefineFont3Tag(h,f,root);
			break;
		case 76:
			ret=new SymbolClassTag(h,f);
			break;
		case 77:
			ret=new MetadataTag(h,f);
			break;
		case 78:
			ret=new DefineScalingGridTag(h,f);
			break;
		case 82:
			ret=new DoABCDefineTag(h,f);
			break;
		case 83:
			ret=new DefineShape4Tag(h,f,root);
			break;
		case 84:
			ret=new DefineMorphShape2Tag(h,f,root);
			break;
		case 86:
			ret=new DefineSceneAndFrameLabelDataTag(h,f);
			break;
		case 87:
			ret=new DefineBinaryDataTag(h,f,root);
			break;
		case 88:
			ret=new DefineFontNameTag(h,f);
			break;
		case 91:
			ret=new DefineFont4Tag(h,f,root);
			break;
		default:
			LOG(LOG_NOT_IMPLEMENTED,_("Unsupported tag type ") << h.getTagType());
			ret=new UnimplementedTag(h,f);
	}

	firstTag=false;

	unsigned int end=f.tellg();
	
	unsigned int actualLen=end-start;
	
	if(actualLen<expectedLen)
	{
		LOG(LOG_ERROR,_("Error while reading tag ") << h.getTagType() << _(". Size=") << actualLen << _(" expected: ") << expectedLen);
		ignore(f,expectedLen-actualLen);
	}
	else if(actualLen>expectedLen)
	{
		LOG(LOG_ERROR,_("Error while reading tag ") << h.getTagType() << _(". Size=") << actualLen << _(" expected: ") << expectedLen);
		throw ParseException("Malformed SWF file");
	}
	
	return ret;
}

RemoveObject2Tag::RemoveObject2Tag(RECORDHEADER h, std::istream& in):DisplayListTag(h)
{
	in >> Depth;
	LOG(LOG_TRACE,_("RemoveObject2 Depth: ") << Depth);
}

void RemoveObject2Tag::execute(DisplayObjectContainer* parent) const
{
	parent->deleteLegacyChildAt(Depth);
}

SetBackgroundColorTag::SetBackgroundColorTag(RECORDHEADER h, std::istream& in):ControlTag(h)
{
	in >> BackgroundColor;
}

DefineEditTextTag::DefineEditTextTag(RECORDHEADER h, std::istream& in, RootMovieClip* root):DictionaryTag(h,root)
{
	//setVariableByName("text",SWFObject(new Integer(0)));
	in >> CharacterID >> Bounds;
	LOG(LOG_TRACE,_("DefineEditTextTag ID ") << CharacterID);
	BitStream bs(in);
	HasText=UB(1,bs);
	WordWrap=UB(1,bs);
	Multiline=UB(1,bs);
	Password=UB(1,bs);
	ReadOnly=UB(1,bs);
	HasTextColor=UB(1,bs);
	HasMaxLength=UB(1,bs);
	HasFont=UB(1,bs);
	HasFontClass=UB(1,bs);
	AutoSize=UB(1,bs);
	HasLayout=UB(1,bs);
	NoSelect=UB(1,bs);
	Border=UB(1,bs);
	WasStatic=UB(1,bs);
	HTML=UB(1,bs);
	UseOutlines=UB(1,bs);
	if(HasFont)
	{
		in >> FontID;
		if(HasFontClass)
			in >> FontClass;
		in >> FontHeight;
		textData.fontSize = FontHeight;
	}
	if(HasTextColor)
		in >> TextColor;
	if(HasMaxLength)
		in >> MaxLength;
	if(HasLayout)
	{
		in >> Align >> LeftMargin >> RightMargin >> Indent >> Leading;
	}
	in >> VariableName;
	LOG(LOG_NOT_IMPLEMENTED,_("Sync to variable name ") << VariableName);
	if(HasText)
	{
		in >> InitialText;
		textData.text = (const char*)InitialText;
	}
	LOG(LOG_NOT_IMPLEMENTED, "DefineEditTextTag does not parse many attributes");
}

ASObject* DefineEditTextTag::instance(Class_base* c) const
{
	if(c==NULL)
		c=Class<TextField>::getClass();
	//TODO: check
	assert_and_throw(bindedTo==NULL);
	TextField* ret=new (c->memoryAccount) TextField(c, textData);
	return ret;
}

DefineSpriteTag::DefineSpriteTag(RECORDHEADER h, std::istream& in, RootMovieClip* root):DictionaryTag(h,root)
{
	in >> SpriteID >> FrameCount;

	LOG(LOG_TRACE,_("DefineSprite ID: ") << SpriteID);
	//Create a non top level TagFactory
	TagFactory factory(in, false);
	Tag* tag;
	bool done=false;
	bool empty=true;
	do
	{
		tag=factory.readTag(root);
		/* We need no locking here, because the vm can only
		 * access this object after construction
		 */
		switch(tag->getType())
		{
			case DICT_TAG:
				throw ParseException("Dictionary tag inside a sprite. Should not happen.");
			case DISPLAY_LIST_TAG:
				addToFrame(static_cast<DisplayListTag*>(tag));
				empty=false;
				break;
			case SHOW_TAG:
			{
				frames.emplace_back(Frame());
				empty=true;
				break;
			}
			case SYMBOL_CLASS_TAG:
			case ABC_TAG:
			case CONTROL_TAG:
				throw ParseException("Control tag inside a sprite. Should not happen.");
			case FRAMELABEL_TAG:
				addFrameLabel(frames.size()-1,static_cast<FrameLabelTag*>(tag)->Name);
				empty=false;
				break;
			case TAG:
				LOG(LOG_NOT_IMPLEMENTED,_("Unclassified tag inside Sprite?"));
				break;
			case END_TAG:
				done=true;
				if(empty && frames.size()!=FrameCount)
					frames.pop_back();
				break;
		}
	}
	while(!done);

	if(frames.size()!=FrameCount)
	{
		//This condition is not critical as Sprites are not executed while being parsed
		LOG(LOG_CALLS,_("Inconsistent frame count in Sprite ID ") << SpriteID);
	}

	setFramesLoaded(frames.size());

	LOG(LOG_TRACE,_("EndDefineSprite ID: ") << SpriteID);
}

ASObject* DefineSpriteTag::instance(Class_base* c) const
{
	Class_base* retClass=NULL;
	if(c)
		retClass=c;
	else if(bindedTo)
		retClass=bindedTo;
	else
		retClass=Class<MovieClip>::getClass();

	return new (retClass->memoryAccount) MovieClip(retClass, *this);
}

void lightspark::ignore(istream& i, int count)
{
	char* buf=new char[count];

	i.read(buf,count);

	delete[] buf;
}

DefineFontTag::DefineFontTag(RECORDHEADER h, std::istream& in, RootMovieClip* root):FontTag(h, 20, root)
{
	LOG(LOG_TRACE,_("DefineFont"));
	in >> FontID;

	UI16_SWF t;
	int NumGlyphs=0;
	in >> t;
	OffsetTable.push_back(t);
	NumGlyphs=t/2;

	for(int i=1;i<NumGlyphs;i++)
	{
		in >> t;
		OffsetTable.push_back(t);
	}

	for(int i=0;i<NumGlyphs;i++)
	{
		SHAPE t;
		in >> t;
		GlyphShapeTable.push_back(t);
	}
}
ASObject* DefineFontTag::instance(Class_base* c) const
{ 
	tiny_string fontname("");
	Class_base* retClass=NULL;
	if(c)
		retClass=c;
	else if(bindedTo)
		retClass=bindedTo;
	else
		retClass=Class<ASFont>::getClass();

	ASFont* ret=new (retClass->memoryAccount) ASFont(retClass);
	LOG(LOG_NOT_IMPLEMENTED,"DefineFontTag::instance doesn't handle all font properties");
	ret->SetFont(fontname,false,false,true,false);
	return ret;
}

DefineFont2Tag::DefineFont2Tag(RECORDHEADER h, std::istream& in, RootMovieClip* root):FontTag(h, 20, root)
{
	LOG(LOG_TRACE,_("DefineFont2"));
	in >> FontID;
	BitStream bs(in);
	FontFlagsHasLayout = UB(1,bs);
	FontFlagsShiftJIS = UB(1,bs);
	FontFlagsSmallText = UB(1,bs);
	FontFlagsANSI = UB(1,bs);
	FontFlagsWideOffsets = UB(1,bs);
	FontFlagsWideCodes = UB(1,bs);
	FontFlagsItalic = UB(1,bs);
	FontFlagsBold = UB(1,bs);
	in >> LanguageCode >> FontNameLen;
	for(int i=0;i<FontNameLen;i++)
	{
		UI8 t;
		in >> t;
		FontName.push_back(t);
	}
	in >> NumGlyphs;
	if(FontFlagsWideOffsets)
	{
		UI32_SWF t;
		for(int i=0;i<NumGlyphs;i++)
		{
			in >> t;
			OffsetTable.push_back(t);
		}
		in >> t;
		CodeTableOffset=t;
	}
	else
	{
		UI16_SWF t;
		for(int i=0;i<NumGlyphs;i++)
		{
			in >> t;
			OffsetTable.push_back(t);
		}
		in >> t;
		CodeTableOffset=t;
	}
	for(int i=0;i<NumGlyphs;i++)
	{
		SHAPE t;
		in >> t;
		GlyphShapeTable.push_back(t);
	}
	if(FontFlagsWideCodes)
	{
		for(int i=0;i<NumGlyphs;i++)
		{
			UI16_SWF t;
			in >> t;
			CodeTable.push_back(t);
		}
	}
	else
	{
		for(int i=0;i<NumGlyphs;i++)
		{
			UI8 t;
			in >> t;
			CodeTable.push_back(t);
		}
	}
	if(FontFlagsHasLayout)
	{
		in >> FontAscent >> FontDescent >> FontLeading;

		for(int i=0;i<NumGlyphs;i++)
		{
			SI16_SWF t;
			in >> t;
			FontAdvanceTable.push_back(t);
		}
		for(int i=0;i<NumGlyphs;i++)
		{
			RECT t;
			in >> t;
			FontBoundsTable.push_back(t);
		}
		in >> KerningCount;
	}
	//TODO: implmented Kerning support
	ignore(in,KerningCount*4);
}
ASObject* DefineFont2Tag::instance(Class_base* c) const
{ 
	tiny_string fontname((const char*)FontName.data(),true);
	Class_base* retClass=NULL;
	if(c)
		retClass=c;
	else if(bindedTo)
		retClass=bindedTo;
	else
		retClass=Class<ASFont>::getClass();

	ASFont* ret=new (retClass->memoryAccount) ASFont(retClass);
	LOG(LOG_NOT_IMPLEMENTED,"DefineFont2Tag::instance doesn't handle all font properties");
	ret->SetFont(fontname,FontFlagsBold,FontFlagsItalic,true,false);
	return ret;
}

DefineFont3Tag::DefineFont3Tag(RECORDHEADER h, std::istream& in, RootMovieClip* root):FontTag(h, 1, root)
{
	LOG(LOG_TRACE,_("DefineFont3"));
	in >> FontID;
	BitStream bs(in);
	FontFlagsHasLayout = UB(1,bs);
	FontFlagsShiftJIS = UB(1,bs);
	FontFlagsSmallText = UB(1,bs);
	FontFlagsANSI = UB(1,bs);
	FontFlagsWideOffsets = UB(1,bs);
	FontFlagsWideCodes = UB(1,bs);
	FontFlagsItalic = UB(1,bs);
	FontFlagsBold = UB(1,bs);
	in >> LanguageCode >> FontNameLen;
	for(int i=0;i<FontNameLen;i++)
	{
		UI8 t;
		in >> t;
		FontName.push_back(t);
	}
	in >> NumGlyphs;
	if(FontFlagsWideOffsets)
	{
		UI32_SWF t;
		for(int i=0;i<NumGlyphs;i++)
		{
			in >> t;
			OffsetTable.push_back(t);
		}
		//CodeTableOffset is missing with zero NumGlyphs
		if(NumGlyphs)
		{
			in >> t;
			CodeTableOffset=t;
		}
	}
	else
	{
		UI16_SWF t;
		for(int i=0;i<NumGlyphs;i++)
		{
			in >> t;
			OffsetTable.push_back(t);
		}
		//CodeTableOffset is missing with zero NumGlyphs
		if(NumGlyphs)
		{
			in >> t;
			CodeTableOffset=t;
		}
	}
	GlyphShapeTable.resize(NumGlyphs);
	for(int i=0;i<NumGlyphs;i++)
	{
		in >> GlyphShapeTable[i];
	}
	for(int i=0;i<NumGlyphs;i++)
	{
		UI16_SWF t;
		in >> t;
		CodeTable.push_back(t);
	}
	if(FontFlagsHasLayout)
	{
		in >> FontAscent >> FontDescent >> FontLeading;

		for(int i=0;i<NumGlyphs;i++)
		{
			SI16_SWF t;
			in >> t;
			FontAdvanceTable.push_back(t);
		}
		for(int i=0;i<NumGlyphs;i++)
		{
			RECT t;
			in >> t;
			FontBoundsTable.push_back(t);
		}
		in >> KerningCount;
	}
	//TODO: implment Kerning support
	ignore(in,KerningCount*4);
}
ASObject* DefineFont3Tag::instance(Class_base* c) const
{ 
	tiny_string fontname((const char*)FontName.data(),true);
	Class_base* retClass=NULL;
	if(c)
		retClass=c;
	else if(bindedTo)
		retClass=bindedTo;
	else
		retClass=Class<ASFont>::getClass();

	ASFont* ret=new (retClass->memoryAccount) ASFont(retClass);
	LOG(LOG_NOT_IMPLEMENTED,"DefineFont3Tag::instance doesn't handle all font properties");
	ret->SetFont(fontname,FontFlagsBold,FontFlagsItalic,true,false);
	return ret;
}

DefineFont4Tag::DefineFont4Tag(RECORDHEADER h, std::istream& in, RootMovieClip* root):DictionaryTag(h,root)
{
	LOG(LOG_TRACE,_("DefineFont4"));
	int dest=in.tellg();
        dest+=h.getLength();

	in >> FontID;
	BitStream bs(in);
	UB(5,bs); /* reserved */
	FontFlagsHasFontData = UB(1,bs);
	FontFlagsItalic = UB(1,bs);
	FontFlagsBold = UB(1,bs);
	in >> FontName;

	if(FontFlagsHasFontData)
		LOG(LOG_NOT_IMPLEMENTED,"DefineFont4Tag with FontData");
	ignore(in,dest-in.tellg());
}
ASObject* DefineFont4Tag::instance(Class_base* c) const
{ 
	tiny_string fontname = FontName;
	Class_base* retClass=NULL;
	if(c)
		retClass=c;
	else if(bindedTo)
		retClass=bindedTo;
	else
		retClass=Class<ASFont>::getClass();

	ASFont* ret=new (retClass->memoryAccount) ASFont(retClass);
	LOG(LOG_NOT_IMPLEMENTED,"DefineFont4Tag::instance doesn't handle all font properties");
	ret->SetFont(fontname,FontFlagsBold,FontFlagsItalic,FontFlagsHasFontData,false);
	return ret;
}

BitmapTag::BitmapTag(RECORDHEADER h,RootMovieClip* root):DictionaryTag(h,root),BitmapContainer(getSys()->tagsMemory)
{
}

DefineBitsLosslessTag::DefineBitsLosslessTag(RECORDHEADER h, istream& in, int version, RootMovieClip* root):BitmapTag(h,root)
{
	int dest=in.tellg();
	dest+=h.getLength();
	in >> CharacterId >> BitmapFormat >> BitmapWidth >> BitmapHeight;

	if(BitmapFormat==3)
		in >> BitmapColorTableSize;

	if(BitmapFormat != 5)
	{
		LOG(LOG_NOT_IMPLEMENTED,"DefineBitsLossless(2)Tag with unsupported BitmapFormat");
		ignore(in,dest-in.tellg());
		return;
	}

	string cData;
	size_t cSize = dest-in.tellg(); //rest of this tag
	cData.resize(cSize);
	in.read(&cData[0], cSize);
	istringstream cDataStream(cData);
	zlib_filter zf(cDataStream.rdbuf());
	istream zfstream(&zf);

	size_t size = BitmapWidth * BitmapHeight * 4;
	uint8_t* inData=new(nothrow) uint8_t[size];
	zfstream.read((char*)inData,size);
	assert(!zfstream.fail() && !zfstream.eof());

	if(version == 1)
	{	/* for version 1, the alpha field is always zero
		 * but should not be interpreted. Setting it to
		 * 0xff (opaque) allows us to handle it as ARGB
		 */
		for(size_t i=0;i<size;i+=4)
			inData[i] = 0xFF;
	}

	fromRGB(inData, BitmapWidth, BitmapHeight, true);
}

ASObject* BitmapTag::instance(Class_base* c) const
{
	Class_base* realClass=(c)?c:bindedTo;
	//Flex imports bitmaps using BitmapAsset as the base class, which is derived from bitmap
	//Also BitmapData is used in the wild though, so support both cases
	bool returnBitmap = false;
	Class_base* classRet = Class<BitmapData>::getClass();
	if(realClass && realClass->isSubClass(Class<Bitmap>::getClass()))
		returnBitmap = true;
	else if(realClass && realClass->isSubClass(Class<BitmapData>::getClass()))
		classRet = realClass;

	BitmapData* ret=new (classRet->memoryAccount) BitmapData(classRet, *this);
	if(returnBitmap)
	{
		Bitmap* bitmapRet=new (realClass->memoryAccount) Bitmap(realClass,_MR(ret));
		return bitmapRet;
	}
	return ret;
}

DefineTextTag::DefineTextTag(RECORDHEADER h, istream& in, RootMovieClip* root,int v):DictionaryTag(h,root),
	tokens(reporter_allocator<GeomToken>(getSys()->tagsMemory)),version(v)
{
	in >> CharacterId >> TextBounds >> TextMatrix >> GlyphBits >> AdvanceBits;
	assert(v==1 || v==2);
	if(v==1)
		LOG(LOG_TRACE,"DefineText ID " << CharacterId);
	else if(v==2)
		LOG(LOG_TRACE,"DefineText2 ID " << CharacterId);

	TEXTRECORD t(this);
	while(1)
	{
		in >> t;
		if(t.TextRecordType+t.StyleFlagsHasFont+t.StyleFlagsHasColor+t.StyleFlagsHasYOffset+t.StyleFlagsHasXOffset==0)
			break;
		TextRecords.push_back(t);
	}
}

ASObject* DefineTextTag::instance(Class_base* c) const
{
	/* we cannot call computeCached in the constructor
	 * because loadedFrom is not available there for dictionary lookups
	 */
	if(tokens.empty())
		computeCached();

	if(c==NULL)
		c=Class<StaticText>::getClass();

	StaticText* ret=new (c->memoryAccount) StaticText(c, tokens);
	return ret;
}

void DefineTextTag::computeCached() const
{
	if(!tokens.empty())
		return;

	const FontTag* curFont = NULL;
	std::list<FILLSTYLE> fillStyles;
	Vector2 curPos;
	FILLSTYLE fs(1);
	fs.FillStyleType = SOLID_FILL;
	fs.Color = RGBA(0,0,0,255);
	fillStyles.push_back(fs);

	for(size_t i=0; i< TextRecords.size();++i)
	{
		if(TextRecords[i].StyleFlagsHasFont)
		{
			const DictionaryTag* it3=loadedFrom->dictionaryLookup(TextRecords[i].FontID);
			curFont=dynamic_cast<const FontTag*>(it3);
			assert_and_throw(curFont);
		}
		assert_and_throw(curFont);
		if(TextRecords[i].StyleFlagsHasColor)
		{
			fillStyles.front().Color = TextRecords[i].TextColor;
		}
		if(TextRecords[i].StyleFlagsHasXOffset)
		{
					curPos.x = TextRecords[i].XOffset;
		}
		if(TextRecords[i].StyleFlagsHasYOffset)
		{
					curPos.y = TextRecords[i].YOffset;
		}
		/*
		 * In DefineFont3Tags, shape's coordinates are 1024*20 times pixels size,
		 * in all former DefineFont*Tags, its just 1024 times. We scale everything here
		 * to 1024*20, so curFont->scaling=20 for DefineFont2Tags and DefineFontTags.
		 * And, of course, scale by the TextHeight.
		 */
		int scaling = TextRecords[i].TextHeight * curFont->scaling;
		for(int j=0;j<TextRecords[i].GlyphCount;++j)
		{
			const GLYPHENTRY& ge = TextRecords[i].GlyphEntries[j];
			const std::vector<SHAPERECORD>& sr = curFont->getGlyphShapes().at(ge.GlyphIndex).ShapeRecords;
			/* curPos is in pixel, but the glyph coordinates are 1024*20 times pixel size,
			 * so we scale curPos. This is scaled back to pixels by cairo.
			 */
			TokenContainer::FromShaperecordListToShapeVector(sr,tokens,fillStyles,curPos*1024*20,scaling);
			curPos.x += ge.GlyphAdvance;
		}
	}
}

DefineShapeTag::DefineShapeTag(RECORDHEADER h,int v,RootMovieClip* root):DictionaryTag(h,root),Shapes(v),
	tokens(reporter_allocator<GeomToken>(getSys()->tagsMemory))
{
}

DefineShapeTag::DefineShapeTag(RECORDHEADER h, std::istream& in,RootMovieClip* root):DictionaryTag(h,root),Shapes(1),
	tokens(reporter_allocator<GeomToken>(getSys()->tagsMemory))
{
	LOG(LOG_TRACE,_("DefineShapeTag"));
	in >> ShapeId >> ShapeBounds >> Shapes;
	TokenContainer::FromShaperecordListToShapeVector(Shapes.ShapeRecords,tokens,Shapes.FillStyles.FillStyles);
}

DefineShape2Tag::DefineShape2Tag(RECORDHEADER h, std::istream& in,RootMovieClip* root):DefineShapeTag(h,2,root)
{
	LOG(LOG_TRACE,_("DefineShape2Tag"));
	in >> ShapeId >> ShapeBounds >> Shapes;
	TokenContainer::FromShaperecordListToShapeVector(Shapes.ShapeRecords,tokens,Shapes.FillStyles.FillStyles);
}

DefineShape3Tag::DefineShape3Tag(RECORDHEADER h, std::istream& in,RootMovieClip* root):DefineShape2Tag(h,3,root)
{
	LOG(LOG_TRACE,"DefineShape3Tag");
	in >> ShapeId >> ShapeBounds >> Shapes;
	TokenContainer::FromShaperecordListToShapeVector(Shapes.ShapeRecords,tokens,Shapes.FillStyles.FillStyles);
}

DefineShape4Tag::DefineShape4Tag(RECORDHEADER h, std::istream& in, RootMovieClip* root):DefineShape3Tag(h,4,root)
{
	LOG(LOG_TRACE,"DefineShape4Tag");
	in >> ShapeId >> ShapeBounds >> EdgeBounds;
	BitStream bs(in);
	UB(5,bs);
	UsesFillWindingRule=UB(1,bs);
	UsesNonScalingStrokes=UB(1,bs);
	UsesScalingStrokes=UB(1,bs);
	in >> Shapes;
	TokenContainer::FromShaperecordListToShapeVector(Shapes.ShapeRecords,tokens,Shapes.FillStyles.FillStyles);
}

DefineMorphShapeTag::DefineMorphShapeTag(RECORDHEADER h, std::istream& in, RootMovieClip* root):DictionaryTag(h, root),
	MorphLineStyles(1)
{
	LOG(LOG_TRACE,"DefineMorphShapeTag");
	UI32_SWF Offset;
	in >> CharacterId >> StartBounds >> EndBounds >> Offset >> MorphFillStyles >> MorphLineStyles >> StartEdges >> EndEdges;
}

ASObject* DefineMorphShapeTag::instance(Class_base* c) const
{
	assert_and_throw(bindedTo==NULL);
	if(c==NULL)
		c=Class<MorphShape>::getClass();
	LOG(LOG_NOT_IMPLEMENTED, _("MorphShape not really supported"));
	return Class<MorphShape>::getInstanceS(c);
}

DefineMorphShape2Tag::DefineMorphShape2Tag(RECORDHEADER h, std::istream& in, RootMovieClip* root):DefineMorphShapeTag(h, root, 2)
{
	LOG(LOG_TRACE,"DefineMorphShape2Tag");
	UI32_SWF Offset;
	in >> CharacterId >> StartBounds >> EndBounds >> StartEdgeBounds >> EndEdgeBounds;
	BitStream bs(in);
	UB(6,bs);
	UsesNonScalingStrokes=UB(1,bs);
	UsesScalingStrokes=UB(1,bs);
	in >> Offset >> MorphFillStyles >> MorphLineStyles >> StartEdges >> EndEdges;
}

//void DefineFont3Tag::genGlyphShape(vector<GeomShape>& s, int glyph)
//{
//	SHAPE& shape=GlyphShapeTable[glyph];
//	FromShaperecordListToShapeVector(shape.ShapeRecords,s);

/*	for(unsigned int i=0;i<s.size();i++)
		s[i].BuildFromEdges(NULL);*/

	//Should check fill state

/*		s.back().graphic.filled0=true;
		s.back().graphic.filled1=false;
		s.back().graphic.stroked=false;

		if(i->state.validFill0)
		{
			if(i->state.fill0!=1)
				LOG(ERROR,_("Not valid fill style for font"));
		}

		if(i->state.validFill1)
		{
			LOG(ERROR,_("Not valid fill style for font"));
		}

		if(i->state.validStroke)
		{
			if(i->state.stroke)
			{
				s.back().graphic.stroked=true;
				LOG(ERROR,_("Not valid stroke style for font"));
//				shapes.back().graphic.stroke_color=Shapes.LineStyles.LineStyles[i->state->stroke-1].Color;
			}
			else
				s.back().graphic.stroked=false;
		}
		else
			s.back().graphic.stroked=false;*/
//}

//void DefineFont2Tag::genGlyphShape(vector<GeomShape>& s, int glyph)
//{
	//SHAPE& shape=GlyphShapeTable[glyph];
	//FromShaperecordListToShapeVector(shape.ShapeRecords,s);

	/*for(unsigned int i=0;i<s.size();i++)
		s[i].BuildFromEdges(NULL);*/

	//Should check fill state

/*		s.back().graphic.filled0=true;
		s.back().graphic.filled1=false;
		s.back().graphic.stroked=false;

		if(i->state.validFill0)
		{
			if(i->state.fill0!=1)
				LOG(ERROR,_("Not valid fill style for font"));
		}

		if(i->state.validFill1)
		{
			LOG(ERROR,_("Not valid fill style for font"));
		}

		if(i->state.validStroke)
		{
			if(i->state.stroke)
			{
				s.back().graphic.stroked=true;
				LOG(ERROR,_("Not valid stroke style for font"));
//				shapes.back().graphic.stroke_color=Shapes.LineStyles.LineStyles[i->state->stroke-1].Color;
			}
			else
				s.back().graphic.stroked=false;
		}
		else
			s.back().graphic.stroked=false;*/
//}

//void DefineFontTag::genGlyphShape(vector<GeomShape>& s,int glyph)
//{
	/*SHAPE& shape=GlyphShapeTable[glyph];
	FromShaperecordListToShapeVector(shape.ShapeRecords,s);

	for(unsigned int i=0;i<s.size();i++)
		s[i].BuildFromEdges(NULL);*/

	//Should check fill state
//}

ShowFrameTag::ShowFrameTag(RECORDHEADER h, std::istream& in):Tag(h)
{
	LOG(LOG_TRACE,_("ShowFrame"));
}

void PlaceObject2Tag::setProperties(DisplayObject* obj, DisplayObjectContainer* parent) const
{
	assert_and_throw(obj && PlaceFlagHasCharacter);

	//TODO: move these three attributes in PlaceInfo
	if(PlaceFlagHasColorTransform)
		obj->ColorTransform=ColorTransform;

	if(PlaceFlagHasRatio)
		obj->Ratio=Ratio;

	if(PlaceFlagHasClipDepth)
		obj->ClipDepth=ClipDepth;

	if(PlaceFlagHasName)
	{
		//Set a variable on the parent to link this object
		LOG(LOG_INFO,_("Registering ID ") << CharacterId << _(" with name ") << Name);
		if(!PlaceFlagMove)
		{
			obj->name = (const char*)Name;
		}
		else
			LOG(LOG_ERROR, _("Moving of registered objects not really supported"));
	}
}

void PlaceObject2Tag::execute(DisplayObjectContainer* parent) const
{
	//TODO: support clipping
	if(ClipDepth!=0)
	{
		LOG(LOG_ERROR,"ClipDepth is not supported");
		return;
	}

	if(!PlaceFlagHasCharacter && !PlaceFlagMove)
	{
		LOG(LOG_ERROR,_("Invalid PlaceObject2Tag that does nothing"));
		return;
	}

	if(PlaceFlagHasCharacter)
	{
		//A new character must be placed
		LOG(LOG_TRACE,_("Placing ID ") << CharacterId);

		if(placedTag==NULL)
			throw RunTimeException("No tag to place");

		//We can create the object right away
		DisplayObject* toAdd=dynamic_cast<DisplayObject*>(placedTag->instance());

		assert_and_throw(toAdd);
		//The matrix must be set before invoking the constructor
		toAdd->setLegacyMatrix(Matrix);

		setProperties(toAdd, parent);

		if( parent->hasLegacyChildAt(Depth) )
		{
			if(PlaceFlagMove)
			{
				parent->deleteLegacyChildAt(Depth);
				/* parent becomes the owner of toAdd */
				parent->insertLegacyChildAt(Depth,toAdd);
			}
			else
				LOG(LOG_ERROR,_("Invalid PlaceObject2Tag that overwrites an object without moving"));
		}
		else
		{
			/* parent becomes the owner of toAdd */
			parent->insertLegacyChildAt(Depth,toAdd);
		}
	}
	else
	{
		parent->transformLegacyChildAt(Depth,Matrix);
	}
}

PlaceObject2Tag::PlaceObject2Tag(RECORDHEADER h, std::istream& in, RootMovieClip* root):DisplayListTag(h),placedTag(NULL)
{
	LOG(LOG_TRACE,_("PlaceObject2"));

	BitStream bs(in);
	PlaceFlagHasClipAction=UB(1,bs);
	PlaceFlagHasClipDepth=UB(1,bs);
	PlaceFlagHasName=UB(1,bs);
	PlaceFlagHasRatio=UB(1,bs);
	PlaceFlagHasColorTransform=UB(1,bs);
	PlaceFlagHasMatrix=UB(1,bs);
	PlaceFlagHasCharacter=UB(1,bs);
	PlaceFlagMove=UB(1,bs);
	in >> Depth;
	if(PlaceFlagHasCharacter)
		in >> CharacterId;

	if(PlaceFlagHasMatrix)
		in >> Matrix;

	if(PlaceFlagHasColorTransform)
		in >> ColorTransform;

	if(PlaceFlagHasRatio)
		in >> Ratio;

	if(PlaceFlagHasName)
		in >> Name;

	if(PlaceFlagHasClipDepth)
		in >> ClipDepth;

	if(PlaceFlagHasClipAction)
		in >> ClipActions;

	assert_and_throw(!(PlaceFlagHasCharacter && CharacterId==0));

	if(PlaceFlagHasCharacter)
		placedTag=root->dictionaryLookup(CharacterId);
}

PlaceObject3Tag::PlaceObject3Tag(RECORDHEADER h, std::istream& in, RootMovieClip* root):PlaceObject2Tag(h)
{
	LOG(LOG_TRACE,_("PlaceObject3"));

	BitStream bs(in);
	PlaceFlagHasClipAction=UB(1,bs);
	PlaceFlagHasClipDepth=UB(1,bs);
	PlaceFlagHasName=UB(1,bs);
	PlaceFlagHasRatio=UB(1,bs);
	PlaceFlagHasColorTransform=UB(1,bs);
	PlaceFlagHasMatrix=UB(1,bs);
	PlaceFlagHasCharacter=UB(1,bs);
	PlaceFlagMove=UB(1,bs);
	UB(3,bs); //Reserved
	PlaceFlagHasImage=UB(1,bs);
	PlaceFlagHasClassName=UB(1,bs);
	PlaceFlagHasCacheAsBitmap=UB(1,bs);
	PlaceFlagHasBlendMode=UB(1,bs);
	PlaceFlagHasFilterList=UB(1,bs);

	in >> Depth;
	if(PlaceFlagHasClassName || (PlaceFlagHasImage && PlaceFlagHasCharacter))
		throw ParseException("ClassName in PlaceObject3 not yet supported");

	if(PlaceFlagHasCharacter)
		in >> CharacterId;

	if(PlaceFlagHasMatrix)
		in >> Matrix;

	if(PlaceFlagHasColorTransform)
		in >> ColorTransform;

	if(PlaceFlagHasRatio)
		in >> Ratio;

	if(PlaceFlagHasName)
		in >> Name;

	if(PlaceFlagHasClipDepth)
		in >> ClipDepth;

	if(PlaceFlagHasFilterList)
		in >> SurfaceFilterList;

	if(PlaceFlagHasBlendMode)
		in >> BlendMode;

	if(PlaceFlagHasCacheAsBitmap)
		in >> BitmapCache;

	if(PlaceFlagHasClipAction)
		in >> ClipActions;

	assert_and_throw(!(PlaceFlagHasCharacter && CharacterId==0))

	if(PlaceFlagHasCharacter)
		placedTag=root->dictionaryLookup(CharacterId);
}

void SetBackgroundColorTag::execute(RootMovieClip* root) const
{
	root->setBackground(BackgroundColor);
}

ProductInfoTag::ProductInfoTag(RECORDHEADER h, std::istream& in):Tag(h)
{
	LOG(LOG_TRACE,_("ProductInfoTag Tag"));

	in >> ProductId >> Edition >> MajorVersion >> MinorVersion >> 
	MinorBuild >> MajorBuild >> CompileTimeLo >> CompileTimeHi;

	uint64_t longlongTime = CompileTimeHi;
	longlongTime<<=32;
	longlongTime|=CompileTimeLo;

	LOG(LOG_INFO,_("SWF Info:") << 
	endl << "\tProductId:\t\t" << ProductId <<
	endl << "\tEdition:\t\t" << Edition <<
	endl << "\tVersion:\t\t" << int(MajorVersion) << "." << int(MinorVersion) << "." << MajorBuild << "." << MinorBuild <<
	endl << "\tCompileTime:\t\t" << longlongTime);
}

FrameLabelTag::FrameLabelTag(RECORDHEADER h, std::istream& in):Tag(h)
{
	in >> Name;
	/* We only support SWF version >=6 */
	UI8 NamedAnchor=in.peek();
	if(NamedAnchor==1)
		in >> NamedAnchor;
}

DefineButton2Tag::DefineButton2Tag(RECORDHEADER h, std::istream& in,RootMovieClip* root):DictionaryTag(h,root)
{
	in >> ButtonId;
	BitStream bs(in);
	UB(7,bs);
	TrackAsMenu=UB(1,bs);
	in >> ActionOffset;

	BUTTONRECORD br(2);

	do
	{
		in >> br;
		if(br.isNull())
			break;
		Characters.push_back(br);
	}
	while(true);

	if(ActionOffset)
		LOG(LOG_NOT_IMPLEMENTED,"DefineButton2Tag: Actions are not supported");
}

ASObject* DefineButton2Tag::instance(Class_base* c) const
{
	DisplayObject* states[4] = {NULL, NULL, NULL, NULL};
	bool isSprite[4] = {false, false, false, false};
	uint32_t curDepth[4];

	/* There maybe multiple DisplayObjects for one state. The official
	 * implementation seems to create a Sprite in that case with
	 * all DisplayObjects as children.
	 */

	auto i = Characters.begin();
	for(;i != Characters.end(); ++i)
	{
		for(int j=0;j<4;++j)
		{
			if(j==0 && !i->ButtonStateDown)
				continue;
			if(j==1 && !i->ButtonStateHitTest)
				continue;
			if(j==2 && !i->ButtonStateOver)
				continue;
			if(j==3 && !i->ButtonStateUp)
				continue;
			const DictionaryTag* dict=loadedFrom->dictionaryLookup(i->CharacterID);

			//We can create the object right away
			DisplayObject* state=dynamic_cast<DisplayObject*>(dict->instance());
			assert_and_throw(state);
			//The matrix must be set before invoking the constructor
			state->setLegacyMatrix(i->PlaceMatrix);
			/*
			 * TODO: BlendMode, filerList, PlaceDepth, ColorTransfrom
			 */
			if(states[j] == NULL)
			{
				states[j] = state;
				curDepth[j] = i->PlaceDepth;
			}
			else
			{
				if(!isSprite[j])
				{
					Sprite* spr = Class<Sprite>::getInstanceS();
					spr->insertLegacyChildAt(curDepth[j],states[j]);
					states[j] = spr;
					spr->name = "Button_spr";
				}
				Sprite* spr = Class<Sprite>::cast(states[j]);
				spr->insertLegacyChildAt(i->PlaceDepth,state);
			}
		}
	}

	if(c==NULL)
		c=Class<SimpleButton>::getClass();
	SimpleButton* ret=new (c->memoryAccount) SimpleButton(c, states[0], states[1], states[2], states[3]);
	return ret;
}

DefineVideoStreamTag::DefineVideoStreamTag(RECORDHEADER h, std::istream& in, RootMovieClip* root):DictionaryTag(h, root)
{
	LOG(LOG_INFO,"DefineVideoStreamTag");
	in >> CharacterID >> NumFrames >> Width >> Height;
	BitStream bs(in);
	UB(4,bs);
	VideoFlagsDeblocking=UB(3,bs);
	VideoFlagsSmoothing=UB(1,bs);
	in >> CodecID;
}

ASObject* DefineVideoStreamTag::instance(Class_base* c) const
{
	Class_base* classRet = NULL;
	if(c)
		classRet=c;
	else if(bindedTo)
		classRet=bindedTo;
	else
		classRet=Class<Video>::getClass();

	Video* ret=new (classRet->memoryAccount) Video(classRet, Width, Height);
	return ret;
}

DefineBinaryDataTag::DefineBinaryDataTag(RECORDHEADER h,std::istream& s,RootMovieClip* root):DictionaryTag(h,root)
{
	LOG(LOG_TRACE,_("DefineBinaryDataTag"));
	int size=h.getLength();
	s >> Tag >> Reserved;
	size -= sizeof(Tag)+sizeof(Reserved);
	bytes=new uint8_t[size];
	len=size;
	s.read((char*)bytes,size);
}

ASObject* DefineBinaryDataTag::instance(Class_base* c) const
{
	uint8_t* b = new uint8_t[len];
	memcpy(b,bytes,len);

	Class_base* classRet = NULL;
	if(c)
		classRet=c;
	else if(bindedTo)
		classRet=bindedTo;
	else
		classRet=Class<ByteArray>::getClass();

	ByteArray* ret=new (classRet->memoryAccount) ByteArray(classRet, b, len);
	return ret;
}

FileAttributesTag::FileAttributesTag(RECORDHEADER h, std::istream& in):Tag(h)
{
	LOG(LOG_TRACE,_("FileAttributesTag Tag"));
	BitStream bs(in);
	UB(1,bs);
	UseDirectBlit=UB(1,bs);
	UseGPU=UB(1,bs);
	HasMetadata=UB(1,bs);
	ActionScript3=UB(1,bs);
	UB(2,bs);
	UseNetwork=UB(1,bs);
	UB(24,bs);
}

DefineSoundTag::DefineSoundTag(RECORDHEADER h, std::istream& in,RootMovieClip* root):DictionaryTag(h,root)
{
	LOG(LOG_TRACE,_("DefineSound Tag"));
	in >> SoundId;
	BitStream bs(in);
	SoundFormat=UB(4,bs);
	SoundRate=UB(2,bs);
	SoundSize=UB(1,bs);
	SoundType=UB(1,bs);
	in >> SoundSampleCount;
	//TODO: read and parse actual sound data
	ignore(in,h.getLength()-7);
}

ASObject* DefineSoundTag::instance(Class_base* c) const
{
	//TODO: use the tag sound data
	return Class<Sound>::getInstanceS(c);
}

ScriptLimitsTag::ScriptLimitsTag(RECORDHEADER h, std::istream& in):ControlTag(h)
{
	LOG(LOG_TRACE,_("ScriptLimitsTag Tag"));
	in >> MaxRecursionDepth >> ScriptTimeoutSeconds;
	LOG(LOG_INFO,_("MaxRecursionDepth: ") << MaxRecursionDepth << _(", ScriptTimeoutSeconds: ") << ScriptTimeoutSeconds);
}

DebugIDTag::DebugIDTag(RECORDHEADER h, std::istream& in):Tag(h)
{
	LOG(LOG_TRACE,_("DebugIDTag Tag"));
	for(int i = 0; i < 16; i++)
		in >> DebugId[i];

	//Note the switch to hex formatting on the ostream, and switch back to dec
	LOG(LOG_INFO,_("DebugId ") << hex <<
		int(DebugId[0]) << int(DebugId[1]) << int(DebugId[2]) << int(DebugId[3]) << "-" <<
		int(DebugId[4]) << int(DebugId[5]) << "-" <<
		int(DebugId[6]) << int(DebugId[7]) << "-" <<
		int(DebugId[8]) << int(DebugId[9]) << "-" <<
		int(DebugId[10]) << int(DebugId[11]) << int(DebugId[12]) << int(DebugId[13]) << int(DebugId[14]) << int(DebugId[15]) <<
		dec);
}

EnableDebuggerTag::EnableDebuggerTag(RECORDHEADER h, std::istream& in):Tag(h)
{
	LOG(LOG_TRACE,_("EnableDebuggerTag Tag"));
	DebugPassword = "";
	if(h.getLength() > 0)
		in >> DebugPassword;
	LOG(LOG_INFO,_("Debugger enabled, password: ") << DebugPassword);
}

EnableDebugger2Tag::EnableDebugger2Tag(RECORDHEADER h, std::istream& in):Tag(h)
{
	LOG(LOG_TRACE,_("EnableDebugger2Tag Tag"));
	in >> ReservedWord;

	DebugPassword = "";
	if(h.getLength() > sizeof(ReservedWord))
		in >> DebugPassword;
	LOG(LOG_INFO,_("Debugger enabled, reserved: ") << ReservedWord << _(", password: ") << DebugPassword);
}

MetadataTag::MetadataTag(RECORDHEADER h, std::istream& in):Tag(h)
{
	LOG(LOG_TRACE,_("MetadataTag Tag"));
	in >> XmlString;
	string XmlStringStd = XmlString;

	try
	{
		xmlpp::TextReader xml((const unsigned char*)XmlStringStd.c_str(), XmlStringStd.length());

		ostringstream output;
		while(xml.read())
		{
			if(xml.get_depth() == 2 && xml.get_node_type() == xmlpp::TextReader::Element)
			{
				output << endl << "\t" << xml.get_local_name() << ":\t\t" << xml.read_string();
			}
		}
		LOG(LOG_INFO, "SWF Metadata:" << output.str());
	}
	catch(std::exception& e)
	{
		LOG(LOG_INFO, "SWF Metadata:" << XmlStringStd);
	}
}

DefineBitsTag::DefineBitsTag(RECORDHEADER h, std::istream& in,RootMovieClip* root):BitmapTag(h,root)
{
	LOG(LOG_TRACE,_("DefineBitsTag Tag"));
	in >> CharacterId;
	//Read image data
	int dataSize=Header.getLength()-2;
	data=new(nothrow) uint8_t[dataSize];
	in.read((char*)data,dataSize);
}

DefineBitsTag::~DefineBitsTag()
{
	delete[] data;
}

DefineBitsJPEG2Tag::DefineBitsJPEG2Tag(RECORDHEADER h, std::istream& in, RootMovieClip* root):BitmapTag(h,root)
{
	LOG(LOG_TRACE,_("DefineBitsJPEG2Tag Tag"));
	in >> CharacterId;
	//Read image data
	int dataSize=Header.getLength()-2;
	uint8_t* inData=new(nothrow) uint8_t[dataSize];
	in.read((char*)inData,dataSize);

	fromJPEG(inData,dataSize);
	delete[] inData;
}

DefineBitsJPEG3Tag::DefineBitsJPEG3Tag(RECORDHEADER h, std::istream& in, RootMovieClip* root):BitmapTag(h,root),alphaData(NULL)
{
	LOG(LOG_TRACE,_("DefineBitsJPEG3Tag Tag"));
	UI32_SWF dataSize;
	in >> CharacterId >> dataSize;
	//Read image data
	uint8_t* inData=new(nothrow) uint8_t[dataSize];
	in.read((char*)inData,dataSize);

	//TODO: check header. Could also be PNG or GIF
	fromJPEG(inData,dataSize);
	delete[] inData;

	//Read alpha data (if any)
	int alphaSize=Header.getLength()-dataSize-6;
	if(alphaSize>0) //If less that 0 the consistency check on tag size will stop later
	{
		//Create a zlib filter
		string alphaData;
		alphaData.resize(alphaSize);
		in.read(&alphaData[0], alphaSize);
		istringstream alphaStream(alphaData);
		zlib_filter zf(alphaStream.rdbuf());
		istream zfstream(&zf);
		zfstream.exceptions ( istream::eofbit | istream::failbit | istream::badbit );

		//Catch the exception if the stream ends
		try
		{
			//Set alpha
			for(int32_t i=0;i<height;i++)
			{
				for(int32_t j=0;j<width;j++)
					data[i*stride + j*4 + 3]=zfstream.get();
			}
		}
		catch(std::exception& e)
		{
			LOG(LOG_ERROR, "Exception while parsing Alpha data in DefineBitsJPEG3");
		}
	}
}

DefineBitsJPEG3Tag::~DefineBitsJPEG3Tag()
{
	delete[] alphaData;
}

DefineSceneAndFrameLabelDataTag::DefineSceneAndFrameLabelDataTag(RECORDHEADER h, std::istream& in):ControlTag(h)
{
	LOG(LOG_TRACE,_("DefineSceneAndFrameLabelDataTag"));
	in >> SceneCount;
	Offset.resize(SceneCount);
	Name.resize(SceneCount);
	for(uint32_t i=0;i<SceneCount;++i)
	{
		in >> Offset[i] >> Name[i];
	}
	in >> FrameLabelCount;
	FrameNum.resize(FrameLabelCount);
	FrameLabel.resize(FrameLabelCount);
	for(uint32_t i=0;i<FrameLabelCount;++i)
	{
		in >> FrameNum[i] >> FrameLabel[i];
	}
}

void DefineSceneAndFrameLabelDataTag::execute(RootMovieClip* root) const
{
	for(size_t i=0;i<SceneCount;++i)
	{
		root->addScene(i,Offset[i],Name[i]);
	}
	for(size_t i=0;i<FrameLabelCount;++i)
	{
		root->addFrameLabel(FrameNum[i],FrameLabel[i]);
	}
}
