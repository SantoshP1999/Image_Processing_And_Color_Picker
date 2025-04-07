#pragma once

class INegative : public IUnknown
{
public:
	// INegative specific method declarations
	virtual HRESULT __stdcall ApplyNegative(COLORREF, COLORREF*) = 0;
};

// CLSID of Negative Component {578603A2-FB1D-4FFD-8669-2AB92C6CE8D5}
const CLSID CLSID_Negative = { 0x578603a2, 0xfb1d, 0x4ffd, 0x86, 0x69, 0x2a, 0xb9, 0x2c, 0x6c, 0xe8, 0xd5 };

//IID of INegative Interface  {125A91FE-1FE2-438F-8622-5F810269D243}
const IID IID_INegative = { 0x125a91fe, 0x1fe2, 0x438f, 0x86, 0x22, 0x5f, 0x81, 0x2, 0x69, 0xd2, 0x43 };

