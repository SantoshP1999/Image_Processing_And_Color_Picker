#pragma once

class IDestauration : public IUnknown
{
public:
	// IDestauration specific method declarations
	virtual HRESULT __stdcall ApplyDestauration(COLORREF, COLORREF*) = 0;	//pure virtual 
};

class ISepia : public IUnknown
{
public:
	// ISepia specific method declarations
	virtual HRESULT __stdcall ApplySepia(COLORREF, COLORREF*) = 0;	//pure virtual 
};

// CLSID of DesaturationSepia Component {B1E1DDE2-3A94-4013-8CCE-357567213D92}
const CLSID CLSID_DesaturationSepia = { 0xb1e1dde2, 0x3a94, 0x4013, 0x8c, 0xce, 0x35, 0x75, 0x67, 0x21, 0x3d, 0x92 };

//IID of IDestauration Interface {802E97C7-E909-40E7-A43C-8F5088DF0FDD}
const IID IID_IDestauration = { 0x802e97c7, 0xe909, 0x40e7, 0xa4, 0x3c, 0x8f, 0x50, 0x88, 0xdf, 0xf, 0xdd };

//IID of ISepia Interface {B5B66752-8AA0-4EC5-A004-3EDADCADC26D}
const IID IID_ISepia = { 0xb5b66752, 0x8aa0, 0x4ec5, 0xa0, 0x4, 0x3e, 0xda, 0xdc, 0xad, 0xc2, 0x6d };

