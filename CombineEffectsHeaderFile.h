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

class INegative : public IUnknown
{
public:
	// INegative specific method declarations
	virtual HRESULT __stdcall ApplyNegative(COLORREF, COLORREF*) = 0;	//pure virtual 
};

// CLSID of DesaturationSepia Component {B1E1DDE2-3A94-4013-8CCE-357567213D92}
const CLSID CLSID_DesaturationSepia = { 0xb1e1dde2, 0x3a94, 0x4013, 0x8c, 0xce, 0x35, 0x75, 0x67, 0x21, 0x3d, 0x92 };

//IID of IDestauration Interface {802E97C7-E909-40E7-A43C-8F5088DF0FDD}
const IID IID_IDestauration = { 0x802e97c7, 0xe909, 0x40e7, 0xa4, 0x3c, 0x8f, 0x50, 0x88, 0xdf, 0xf, 0xdd };

//IID of ISepia Interface {B5B66752-8AA0-4EC5-A004-3EDADCADC26D}
const IID IID_ISepia = { 0xb5b66752, 0x8aa0, 0x4ec5, 0xa0, 0x4, 0x3e, 0xda, 0xdc, 0xad, 0xc2, 0x6d };

// CLSID of Negative Component {578603A2-FB1D-4FFD-8669-2AB92C6CE8D5}
const CLSID CLSID_Negative = { 0x578603a2, 0xfb1d, 0x4ffd, 0x86, 0x69, 0x2a, 0xb9, 0x2c, 0x6c, 0xe8, 0xd5 };

//IID of INegative Interface  {125A91FE-1FE2-438F-8622-5F810269D243}
const IID IID_INegative = { 0x125a91fe, 0x1fe2, 0x438f, 0x86, 0x22, 0x5f, 0x81, 0x2, 0x69, 0xd2, 0x43 };