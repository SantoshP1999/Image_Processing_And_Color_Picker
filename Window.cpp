
/*

        PROJECT NAME : Image Processing And Color Picker
        Developed By : Santosh Balu Phatangare
        Email : santoshphatangare17@gmail.com
        Technologies used : Win32 SDK
                            COM (Component Object Model)

        Programming Language : C++ Programming Language

*/


#include<windows.h>
#include<commdlg.h>
#include<stdio.h>
#include<stdlib.h>
#include<malloc.h>
#include<ctype.h>
#include<string.h>
#include<windowsx.h>

#include<sapi.h>
#include<sphelper.h>

#include "Window.h"
#include "CommonDialog.h"
#include "CombineEffectsHeaderFile.h"

#define UNICODE
#pragma comment(lib, "comdlg32.lib")

//Global Callback Declaration
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK MyDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK RegisterDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK AboutDlgProc(HWND, UINT, WPARAM, LPARAM);

BOOL ValidateString(TCHAR*);

//Log Files
BOOL CreateUserLogFile(TCHAR*);
BOOL CreatePickedColorLogFile(TCHAR*);
BOOL CreateNormalizedColorLogFile(TCHAR*);

void UnInitializeLogFiles();


//Golbal variable declaration for Spot Validation
BOOL bCatchFirstName = TRUE;
BOOL bCatchMiddleName = TRUE;
BOOL bCatchLastName = TRUE;
BOOL bNotNullOrEmptyString = FALSE;
BOOL bColorPickerOn = FALSE;

//Golbal variable declaration for modeless dialog box
HWND hwndModeless;

//Golbal variable declaration for Effect Apply
BOOL bApplyDesaturation = FALSE;
BOOL bApplySepia = FALSE;
BOOL bApplyColorInversion = FALSE;
BOOL bApplyOriginalColor = FALSE;

//Pointers for COM
IDestauration* pIdesaturation = NULL;
ISepia* pIsepia = NULL;
INegative* pInegative = NULL;

//Global file pointer for log files
FILE* gpFile_UserLog = NULL;
FILE* gpFile_ExportPickedColors = NULL;
FILE* gpFile_NormalisedPickedColors = NULL;

//Global Variable Declration For Color Picker
BOOL bLeftClickForColorPicker = FALSE;
BOOL bExportPickedColors = FALSE;
BOOL bNormalisedPickedColors = FALSE;

//Pixel Color
unsigned int pickedPixelColorR = 0;
unsigned int pickedPixelColorG = 0;
unsigned int pickedPixelColorB = 0;

//Nomalised Pixel Color
float normalizedRed = 0.0f;
float normalizedGreen = 0.0f;
float normalizedBlue = 0.0f;

static LPSTR  lpszFirstName = NULL;
static LPSTR  lpszMiddleName = NULL;
static LPSTR  lpszLastName = NULL;

TCHAR  strFirstName[256] = { '\0' };
TCHAR  strMiddleName[256] = { '\0' };
TCHAR  strLastName[256] = { '\0' };
TCHAR str[256] = { '\0' };

//Window Width & Height
int WIDTH = GetSystemMetrics(SM_CXSCREEN) / 2;
int HEIGHT = GetSystemMetrics(SM_CYSCREEN) / 2;

//Button
HWND hButton;

//  *********************   SAPI    **************************

ISpVoice* pISpVoice = NULL; 
HWND hListBox;

HRESULT hr;
ISpObjectToken* voices[2];
//ISpObjectToken* pISpObjectToken = NULL;
ISpObjectToken* pISpObjectToken1 = NULL;
ISpObjectToken* pISpObjectToken2 = NULL;

IEnumSpObjectTokens* pIEnumSpObjectTokens = NULL;

ISpStream* pISpStream = NULL; 
CSpStreamFormat audioFormat;   

WCHAR* descriptionString = NULL;

//int iPaintFlag = 0;
int selectedVoiceIndex = -1;

BOOL g_UserRegistered = FALSE;
BOOL g_dataRegister = FALSE;
BOOL g_bRadioButtonChecked = FALSE;
BOOL g_CheckBoxChecked = FALSE;
BOOL g_ListBox = FALSE;
BOOL soundOff = FALSE;

//BOOL setCheck = TRUE;
//BOOL bAudioLog = FALSE;

//BOOL isChecked = FALSE;
//int isChecked1 = 0;
//USHORT CurrentVolume = 100.0f;

//BOOL EnableExportRadioButton = FALSE; 

//  **********************************************************

//Local Time
SYSTEMTIME static time;
//User Name
TCHAR strUserLogName[512] = { '\0' };

//Entry-point Function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
    //variable declarations 
    WNDCLASSEX wndclass;
    HWND hwnd;
    HRESULT hr = S_OK; 
    MSG msg;
    TCHAR szClassName[] = TEXT("OPEN_DIALOG_BOX_WINDOW");
    HRESULT hResult = S_OK;

    //code 
    hResult = CoInitialize(NULL);

    if (FAILED(hResult))
    {
        MessageBox(NULL, "CoInitilise Failed", "COM ERROR", MB_OK | MB_ICONERROR);
        exit(0);
    }

    ZeroMemory((void*)&wndclass, sizeof(WNDCLASSEX));

    //Initializing Window Class
    wndclass.cbSize = sizeof(WNDCLASSEX);
    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.lpfnWndProc = WndProc;
    wndclass.lpszClassName = szClassName;
    wndclass.lpszMenuName = "IMAGE_EDITOR";
    wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wndclass.hInstance = hInstance;
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(APP_ICON));
    wndclass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(APP_ICON));


    //Register The Above Window Class
    RegisterClassEx(&wndclass);
   
    //Create The Window In Memory
    hwnd = CreateWindow(szClassName,
        TEXT("Image Processing And Color Picker"),
        WS_MINIMIZEBOX|WS_CAPTION| WS_SYSMENU,
        //WS_OVERLAPPED,
        0,
        0,
        WIDTH,
        HEIGHT,
        NULL,
        NULL,
        hInstance,
        NULL);


    ShowWindow(hwnd, iCmdShow);

    UpdateWindow(hwnd);

    //Message Loop
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    CoUninitialize();
    return((int)msg.wParam);
}

//Window Procedure 
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{ 
    //function declarations
    void SafeInterfaceRelease(void);

    //variable declarations
    static int cxClient, cyClient;
    HDC hdc, hdcMem;
    static HINSTANCE hIns = NULL;
    RECT rect;
    OPENFILENAME openFileName;
    int i, j, y;
    static HBITMAP hBitMapFile = NULL;
    BITMAP bm;
    PAINTSTRUCT    ps;
    TCHAR szFileName[_MAX_PATH];
    TCHAR STR[255];
    unsigned int xColumn = 0, yRow = 0;
    static unsigned int pickedPixelXCoord = 0, pickedPixelYCoord = 0;
    HRESULT hResult = S_OK;
    BITMAP bitmap = { 0 };
    int retVal = 0;

    SYSTEMTIME stdTime;
    GetLocalTime(&stdTime);

    static HCURSOR hHandCursor;

    static BOOL hideText = FALSE; 

    //  *********************************
    HRESULT hr = S_OK;
    HINSTANCE hInst = NULL;
    ULONG numVoices = 0l;
    WCHAR* DescriptionString = NULL;

    //  *********************************

//code
switch (iMsg)
{
case WM_CREATE:

    hButton = CreateWindow("BUTTON", "Register User", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 250, 110, 130, 30, hwnd, (HMENU)BUTTON_ID, ((LPCREATESTRUCT)lParam)->hInstance, NULL);  

    hHandCursor = LoadCursor(NULL, IDC_HAND);
   
    
    EnableMenuItem(GetMenu(hwnd), IDM_OPEN_FILE, MF_GRAYED);
    EnableMenuItem(GetMenu(hwnd), IDM_EDIT_IMAGE, MF_GRAYED);

    hIns = (HINSTANCE)((LPCREATESTRUCT)lParam)->hInstance;

    hResult = CoCreateInstance(CLSID_DesaturationSepia, NULL, CLSCTX_INPROC_SERVER, IID_IDestauration, (void**)&pIdesaturation);

    //  ***********************************
    hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_INPROC_SERVER, IID_ISpVoice, (void**)&pISpVoice);
    if (FAILED(hr))
    {
        MessageBox(NULL, TEXT("ISpVoice Interface can not be Obtained"), TEXT("ERROR"), MB_ICONERROR);
        DestroyWindow(hwnd);
    }


    pISpVoice->Speak(L"Click On Register User Button For Registration", SPF_ASYNC, NULL);
   
    //  ************************************

    if (FAILED(hResult))
    {
        MessageBox(NULL, "IDesaturation interface could not be obtained", "COM ERROR", MB_OK | MB_ICONERROR);
        DestroyWindow(hwnd);
    }
   
    hResult = pIdesaturation->QueryInterface(IID_ISepia, (void**)&pIsepia);

    if (FAILED(hResult))
    {
        MessageBox(NULL, "ISepia interface could not be obtained", "COM ERROR", MB_OK | MB_ICONERROR);
        DestroyWindow(hwnd);
    }

    hResult = pIsepia->QueryInterface(IID_INegative, (void**)&pInegative);

    if (FAILED(hResult))
    {
        MessageBox(NULL, "INegative interface could not be obtained", "COM ERROR", MB_OK | MB_ICONERROR);
        DestroyWindow(hwnd);
    }

   
    break;
     
case WM_SETCURSOR:
    if (LOWORD(lParam) == HTCLIENT)
    {
        SetCursor(hHandCursor); 
        return TRUE;
    }
    break;
           
break;


case WM_SIZE:
  
    cxClient = LOWORD(lParam);
    cyClient = HIWORD(lParam);
  
    break;

case WM_PAINT:
{
    hdc = BeginPaint(hwnd, &ps);
    hdcMem = CreateCompatibleDC(hdc);

    cxClient = (cxClient >= WIDTH) ? WIDTH : cxClient;
    cyClient = (cyClient >= HEIGHT) ? HEIGHT : cyClient;

    if (!hBitMapFile && !hideText) 
    {
        SetBkColor(hdc, RGB(0, 0, 0));
        SetTextColor(hdc, RGB(0, 255, 0));//Green 
        TextOut(hdc, 150, 155, "Click On  \'Register User\'  Button For Registration.", 50);
        //TextOut(hdc, 150, 200, "Click On File menu.", 20);
    }
    else
    {
        SetBkColor(hdc, RGB(0, 0, 0)); 
        SetTextColor(hdc, RGB(0, 255, 0));//Green  
        TextOut(hdc, 150, 155, "Click On File menu to Select an Image for Processing.", 55); 
    }
    

    GetObject(hBitMapFile, sizeof(BITMAP), (PTSTR)&bm);//Get Dimensions of bitmap to be stretched
    SelectObject(hdcMem, hBitMapFile);
    SetFocus(hwnd);
    SetStretchBltMode(hdcMem, COLORONCOLOR);
    StretchBlt(hdc, 0, 0, cxClient, cyClient, hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
    COLORREF originalPixelColor = GetPixel(hdc, xColumn, yRow);

    if (hBitMapFile)
    {
        if (bApplyDesaturation == TRUE)
        {
            for (yRow = 0; yRow < cyClient; yRow++)//This is one row or every row
            {
                for (xColumn = 0; xColumn < cxClient; xColumn++)//This is column from each row
                {
                    //Get Color from the pixel at co-ordinate (xColumn, yRow)
                    originalPixelColor = GetPixel(hdc, xColumn, yRow);
                    COLORREF desaturatedPixelColor = NULL;
                    //pass originalPixelColor to COM function
                    pIdesaturation->ApplyDestauration(originalPixelColor, &desaturatedPixelColor);
                    SetPixel(hdc, xColumn, yRow, desaturatedPixelColor);
                }
            }
        }
        bApplyDesaturation = FALSE;

        if (bApplySepia == TRUE)
        {
            for (yRow = 0; yRow < cyClient; yRow++)//This is one row or every row
            {
                for (xColumn = 0; xColumn < cxClient; xColumn++)//This is column from each row
                {
                    //Get Color from the pixel at co-ordinate (xColumn, yRow)
                    originalPixelColor = GetPixel(hdc, xColumn, yRow);
                    COLORREF sepiaPixelColor = NULL;
                    //pass originalPixelColor to COM function
                    pIsepia->ApplySepia(originalPixelColor, &sepiaPixelColor);

                    SetPixel(hdc, xColumn, yRow, sepiaPixelColor);
                }
            }
        }
        bApplySepia = FALSE;

        if (bApplyColorInversion == TRUE)
        {
            for (yRow = 0; yRow < cyClient; yRow++)//This is one row or every row
            {
                for (xColumn = 0; xColumn < cxClient; xColumn++)//This is column from each row
                {
                    //Get Color from the pixel at co-ordinate (xColumn, yRow)
                    originalPixelColor = GetPixel(hdc, xColumn, yRow);
                    COLORREF negativePixelColor = NULL;
                    //pass originalPixelColor to COM function
                    pInegative->ApplyNegative(originalPixelColor, &negativePixelColor);

                    SetPixel(hdc, xColumn, yRow, negativePixelColor);
                }
            }
        }
        bApplyColorInversion = FALSE;

    }
    

    if (bApplyOriginalColor)
    {
        SetPixel(hdc, xColumn, yRow, originalPixelColor);

        bApplyOriginalColor = FALSE;
    }

    if (bLeftClickForColorPicker == TRUE)
    {
        COLORREF pickedPixelColorRGB = GetPixel(hdc, pickedPixelXCoord, pickedPixelYCoord);

        pickedPixelColorR = GetRValue(pickedPixelColorRGB);
        pickedPixelColorG = GetGValue(pickedPixelColorRGB);
        pickedPixelColorB = GetBValue(pickedPixelColorRGB);


        normalizedRed = (float)pickedPixelColorR / 255.0f;
        normalizedGreen = (float)pickedPixelColorG / 255.0f;
        normalizedBlue = (float)pickedPixelColorB / 255.0f;

        SetDlgItemInt(hwndModeless, R_TEXT_BOX, pickedPixelColorR, FALSE);
        SetDlgItemInt(hwndModeless, G_TEXT_BOX, pickedPixelColorG, FALSE);
        SetDlgItemInt(hwndModeless, B_TEXT_BOX, pickedPixelColorB, FALSE);

        SendMessage(FindWindow(NULL, TEXT("Image Processing And Color Picker")), WM_USER + 1, (WPARAM)1, (LPARAM)pickedPixelColorRGB);

        bLeftClickForColorPicker = FALSE; 

        pISpVoice->Speak(L"User Picked Color", SPF_ASYNC, NULL); 

        if (bExportPickedColors)
        {
             fprintf(gpFile_ExportPickedColors, "User Picked color RGB = (%u, %u, %u)\n", pickedPixelColorR, pickedPixelColorG, pickedPixelColorB);
             fprintf(gpFile_UserLog, "%02d:%02d:%02d - User Picked Color.\n", stdTime.wHour, stdTime.wMinute, stdTime.wSecond);
        }
          

        if (bNormalisedPickedColors)
        {   
            fprintf(gpFile_NormalisedPickedColors, "User Picked color normalised RGB = (%.2f, %.2f, %.2f)\n", normalizedRed, normalizedGreen, normalizedBlue);
           // fprintf(gpFile_UserLog, "%02d:%02d:%02d - User Picked Normalized Color.\n", stdTime.wHour, stdTime.wMinute, stdTime.wSecond);

        }
           

    }
    DeleteDC(hdcMem);
    EndPaint(hwnd, &ps);
}
    break;

case WM_COMMAND:

    if (LOWORD(wParam) == BUTTON_ID)
    {
        pISpVoice->Speak(L"Please Enter The User Details", SPF_ASYNC, NULL);
        DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(REGISTER_DIALOG), hwnd, RegisterDlgProc); 
        fprintf(gpFile_UserLog, "%02d:%02d:%02d - User Clicked On Register Button.\n", stdTime.wHour, stdTime.wMinute, stdTime.wSecond);
        ShowWindow(hButton, SW_HIDE);
        hideText = TRUE;
       // InvalidateRect(hwnd, NULL, TRUE); 
    }
    
    switch (LOWORD(wParam))
    {
    case IDM_OPEN_FILE:
        ZeroMemory((void*)&openFileName, sizeof(OPENFILENAME));
        ZeroMemory(szFileName, _MAX_PATH);

        openFileName.lStructSize = sizeof(OPENFILENAME);
        openFileName.hwndOwner = hwnd;
        openFileName.lpstrFile = szFileName;
        openFileName.nMaxFile = sizeof(szFileName);
        openFileName.lpstrFilter = TEXT("BMP Files (*.BMP; *.jpg; *.jpeg; *.gif)\0*.BMP; *.jpg\0");
        openFileName.nFilterIndex = 1;
        openFileName.lpstrFileTitle = NULL;
        openFileName.nMaxFileTitle = 0;
        openFileName.lpstrInitialDir = NULL;
        openFileName.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER;
       

        if (GetOpenFileName((LPOPENFILENAME)&openFileName) == FALSE)
        {
            if (CommDlgExtendedError() != 0)
            {
                MessageBox(hwnd, TEXT("Common Dialog Box Error While Opening File."), NULL, MB_OK);
                DestroyWindow(hwnd);
            }
            break;
        }
        
        if (!hBitMapFile)
        {
            DeleteObject(hBitMapFile);
            hBitMapFile = NULL;
            hBitMapFile = (HBITMAP)LoadImage(hIns, openFileName.lpstrFile, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
        }
        else
            hBitMapFile = (HBITMAP)LoadImage(hIns, openFileName.lpstrFile, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
       
       
        if (hBitMapFile == NULL)
        {
            wsprintf(STR, TEXT("File Opening Error Occurs While Opening The File %s"), openFileName.lpstrFile);
            MessageBox(hwnd, STR, NULL, MB_OK);
            break;
        } 

        pISpVoice->Speak(L"Image Opened Successfully", SPF_ASYNC, NULL);
      
        InvalidateRect(hwnd, NULL, TRUE);
        break;

    case IDM_EDIT_IMAGE:
        //pISpVoice->Speak(L"Dialog Box Opened", SPF_ASYNC, NULL);
        hwndModeless = CreateDialog(hIns, MAKEINTRESOURCE(MY_DIALOG), hwnd, MyDlgProc);
        InvalidateRect(hwnd, NULL, TRUE);
        break;

    case IDM_ABOUT:
        pISpVoice->Speak(L"Hello !!! Welcome To My Profile ", SPF_ASYNC, NULL);
        DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(ABOUT_DIALOG), hwnd, AboutDlgProc);
        break;
    
    case IDM_EXIT_APP:
       // pISpVoice->Speak(L"User Clicked On About Button", SPF_ASYNC, NULL);
        pISpVoice->Speak(L"Do You Want To Exit The Application", SPF_ASYNC, NULL); 
        retVal = MessageBox(NULL, TEXT("Do You Want To Exit The Application?"), TEXT("EXIT APPLICATION"), MB_YESNO | MB_ICONQUESTION); 

        switch (retVal)
        {
        case IDYES:
            DestroyWindow(hwnd);
            break;
        case IDNO:
           // EndDialog(hwnd, IDM_EXIT_APP);
            break;
        }
        
    }
    break;
case WM_LBUTTONDOWN:
    if (bColorPickerOn && hBitMapFile)
    {
        bLeftClickForColorPicker = TRUE;
        pickedPixelXCoord = GET_X_LPARAM(lParam);
        pickedPixelYCoord = GET_Y_LPARAM(lParam);
        InvalidateRect(hwnd, NULL, TRUE);

        ////SetCursor(hHandCursor); 
        //if (LOWORD(lParam) == HTCLIENT) 
        //{
           // SetCursor(hHandCursor2);   
        //    return TRUE; 
        //}

    }
    break;
case WM_CLOSE:

    DestroyWindow(hwnd);
   
    break;
case WM_DESTROY:
    if (hBitMapFile)
        DeleteObject(hBitMapFile);
    
    UnInitializeLogFiles();

    //fclose(gpFile_ExportPickedColors);
    //gpFile_ExportPickedColors = NULL;

    //fclose(gpFile_NormalisedPickedColors);
    //gpFile_NormalisedPickedColors = NULL; 

    //fclose(gpFile_UserLog);
    //gpFile_UserLog = NULL;



    SafeInterfaceRelease();
    PostQuitMessage(0);

    break;

default:
    break;
}

// Calling Default Window Procedure
return(DefWindowProc(hwnd, iMsg, wParam, lParam));//Default window procedure
}


//Main Dialog Box Callback Procedure
INT_PTR CALLBACK MyDlgProc(HWND hEditImageDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    //variable declarations
    HDC hdc = NULL;
    HDC hdcStatic = NULL;
    HBRUSH hBrushDlgBk = NULL;
    HBRUSH hBrushTextBk = NULL;
    int iImageProcessorRd = 1;
    int iColorPickerRd = 0;
    static unsigned int pickedPixelXCoord = 0, pickedPixelYCoord = 0;
    TCHAR str[255];
    SYSTEMTIME localTime; 
    GetLocalTime(&localTime); 
    int retVal = 0;

    static COLORREF colorPickerBox; 
    static HBRUSH hBrushColorPicker; 
    static HWND hColorPickerBox = NULL;

    //  **********************************
    HRESULT hr;
    HINSTANCE hInst = NULL;
    //HWND hListBox;
    //HWND ListBox;
    CSpStreamFormat audioFormat;
    ULONG numVoices = 0l;
    WCHAR* DescriptionString = NULL;

    BOOL setCheck = TRUE;
    HWND hCheckBox;
    HWND hwndCaption;
    HWND hwndGroupBox;
    //HDC hdc;
    //  *********************************

    switch (iMsg) 
    {
    case WM_INITDIALOG:

        // Handle To Color Picker Box
        // hColorPickerBox = GetDlgItem(hEditImageDlg, ID_EDIT_COLOR);
       
        //Set Focus in name
        SetFocus(GetDlgItem(hEditImageDlg, 1));
        //SendDlgItemMessage(hEditImageDlg, RD_IMAGE_PROCESSING, BM_SETCHECK, 1, 0);
        EnableWindow(GetDlgItem(hEditImageDlg, ID_IMAGE_SETS), FALSE);
        EnableWindow(GetDlgItem(hEditImageDlg, MY_IMAGE_EDITOR), FALSE);
        EnableWindow(GetDlgItem(hEditImageDlg, IMAGE_DESATURATION), FALSE);
        EnableWindow(GetDlgItem(hEditImageDlg, IMAGE_COLOR_INVERSION), FALSE);
        EnableWindow(GetDlgItem(hEditImageDlg, RD_IMAGE_PROCESSING), FALSE);
        EnableWindow(GetDlgItem(hEditImageDlg, RD_COLOR_PICKER), FALSE);
        EnableWindow(GetDlgItem(hEditImageDlg, IMAGE_SEPIA), FALSE);
        EnableWindow(GetDlgItem(hEditImageDlg, DESATURATION_PUSHBUTTON), FALSE);
        EnableWindow(GetDlgItem(hEditImageDlg, DESATURATION_RESET), FALSE);
        EnableWindow(GetDlgItem(hEditImageDlg, SEPIA_PUSHBUTTON), FALSE);
        EnableWindow(GetDlgItem(hEditImageDlg, SEPIA_RESET), FALSE);
        EnableWindow(GetDlgItem(hEditImageDlg, COLOR_INVERSION_PUSHBUTTON), FALSE);
        EnableWindow(GetDlgItem(hEditImageDlg, COLOR_INVERSION_RESET), FALSE);
        EnableWindow(GetDlgItem(hEditImageDlg, COLOR_PICKER), FALSE);
        EnableWindow(GetDlgItem(hEditImageDlg, R_TEXT_BOX), FALSE);
        EnableWindow(GetDlgItem(hEditImageDlg, G_TEXT_BOX), FALSE);
        EnableWindow(GetDlgItem(hEditImageDlg, B_TEXT_BOX), FALSE);
        EnableWindow(GetDlgItem(hEditImageDlg, PICKED_COLOR), FALSE);
        EnableWindow(GetDlgItem(hEditImageDlg, RED_VALUE), FALSE);
        EnableWindow(GetDlgItem(hEditImageDlg, GREEN_VALUE), FALSE);
        EnableWindow(GetDlgItem(hEditImageDlg, BLUE_VALUE), FALSE);
        EnableWindow(GetDlgItem(hEditImageDlg, COLOR_PICKER_BOX), FALSE);
        EnableWindow(GetDlgItem(hEditImageDlg, EXPORT_RGB_CHECK), FALSE);
        EnableWindow(GetDlgItem(hEditImageDlg, NORMALISE_RGB_CHECK), FALSE);

        //  *************************************
        
        hr = pISpVoice->SetVolume(100.0f);

        //SendDlgItemMessage(hEditImageDlg, ID_VOLUME_OFF, BM_SETCHECK, 1, 0);
        SendDlgItemMessage(hEditImageDlg, ID_VOLUME_ON, BM_SETCHECK, 1, 0);

        //Disable Export Audio log Button Initially
        //EnableExportRadioButton = FALSE;

        //Get a List Of Voice Tokens
        hr = SpEnumTokens(SPCAT_VOICES, NULL, NULL, &pIEnumSpObjectTokens);
        if (FAILED(hr))
        {
            MessageBox(NULL, TEXT("IEnumSpObjectTokens Interface can not be Obtained"), TEXT("ERROR"), MB_OK);
            //DestroyWindow(hwnd);
        }

        //From the list ofvoice tokens get the next voice tokens
        pIEnumSpObjectTokens->Next(1, &pISpObjectToken1, NULL);
        SpGetDescription(pISpObjectToken1, &descriptionString);

        //List  Box Control
        hListBox = GetDlgItem(hEditImageDlg, ID_LISTBOX);
        SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)"Microsoft David");
        SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)"Microsoft Zira");

        //Set the current voice to the voice given in the above obtained Tokens
        pISpVoice->SetVoice(pISpObjectToken1);

        pIEnumSpObjectTokens->Next(1, &pISpObjectToken2, NULL);
        SpGetDescription(pISpObjectToken2, &descriptionString);


        //hListBox = GetDlgItem(hEditImageDlg, ID_LISTBOXEXPORT);
        //SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)"Microsoft David");
        //SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)"Microsoft Zira");
        //  *************************************

        return(TRUE);

    case WM_COMMAND:

        //  ********************************************
        if (HIWORD(wParam) == BN_CLICKED)
        {
            switch (LOWORD(wParam))
            {
            case ID_VOLUME_ON:
                hr = pISpVoice->SetVolume(100.0f);
                SendDlgItemMessage(hEditImageDlg, ID_VOLUME_ON, BM_SETCHECK, TRUE, 0);
                SendDlgItemMessage(hEditImageDlg, ID_VOLUME_OFF, BM_SETCHECK, FALSE, 0);
                EnableWindow(GetDlgItem(hEditImageDlg, ID_LISTBOX), TRUE);
                //EnableExportRadioButton = TRUE;
                //fprintf(LogFile, "Audio On\n");
                break;

            case ID_VOLUME_OFF:
                hr = pISpVoice->SetVolume(0.0f);
                SendDlgItemMessage(hEditImageDlg, ID_VOLUME_OFF, BM_SETCHECK, TRUE, 0);
                SendDlgItemMessage(hEditImageDlg, ID_VOLUME_ON, BM_SETCHECK, FALSE, 0);
                EnableWindow(GetDlgItem(hEditImageDlg, ID_LISTBOX), FALSE);
                //EnableExportRadioButton = FALSE;
                //fprintf(LogFile, "Audio Off\n");
                break;

            }
        }

        //  *******************************************

        switch (LOWORD(wParam))
        {
           
        case IDOK:
               
            if(gpFile_UserLog)
                pISpVoice->Speak(L"Closing The Dialog Box", SPF_ASYNC, NULL);
                  fprintf(gpFile_UserLog, "%02d:%02d:%02d - Closing Edit Image and Picked Color Dialog Box.\n", localTime.wHour, localTime.wMinute, localTime.wSecond);
            
            EndDialog(hEditImageDlg, IDOK);   
        break;

        case IDCANCEL:
            pISpVoice->Speak(L"Do You Want To Exit The Application", SPF_ASYNC, NULL);
            retVal = MessageBox(NULL, TEXT("Do You Want To Exit The Application?"), TEXT("EXIT APPLICATION"), MB_YESNO | MB_ICONQUESTION); 

            switch (retVal)
            {
            case IDYES:
               // fprintf(gpFile_UserLog, "User closed Color.\n", time.wHour, time.wMinute, time.wSecond);

                DestroyWindow(GetParent(hEditImageDlg));
                break;
            case IDNO:
                //EndDialog(hEditImageDlg, IDCANCEL);
                break;
            }
           /* if (gpFile_UserLog)
                fprintf(gpFile_UserLog, "Closing Edit Image and Picked Color Dialog Box.\n");
            
            EndDialog(hEditImageDlg, IDCANCEL);*/

        break;

        //  **********************************************

        case ID_LISTBOX:
            switch (HIWORD(wParam))
            {
            case LBN_SELCHANGE:

                int selectedIndex = SendMessage(GetDlgItem(hEditImageDlg, ID_LISTBOX), LB_GETCURSEL, 0, 0);

                if (selectedIndex != LB_ERR)
                {
                    if (selectedIndex == 0)
                    {
                        pISpVoice->SetVoice(pISpObjectToken1);
                        selectedIndex++;
                    }
                    else if (selectedIndex == 1)
                    {
                        pISpVoice->SetVoice(pISpObjectToken2);
                    }
                }
                break;
            }
            break;

        //case ID_CHECK_EXP_LOG:

        //    switch (HIWORD(wParam))
        //    {
        //    case BN_CLICKED:
        //        EnableWindow(GetDlgItem(hEditImageDlg, ID_LISTBOXEXPORT), TRUE);
        //        SendMessage(GetDlgItem(hEditImageDlg, ID_CHECK_EXP_LOG), BM_SETCHECK, BST_CHECKED, 0);
        //        break;
        //    }
        //    break;

        //case ID_LISTBOXEXPORT:
        //    switch (HIWORD(wParam))
        //    {
        //    case LBN_SELCHANGE:
        //        int selectedIndex = SendMessage(GetDlgItem(hEditImageDlg, ID_LISTBOXEXPORT), LB_GETCURSEL, 0, 0);
        //        if (selectedIndex != LB_ERR)
        //        {
        //            convertTextToSpeech = TRUE;
        //            if (selectedIndex == 0)
        //            {
        //                SetVoiceForAudioLog = 1;
        //                //pISpVoice->SetVoice(pISpObjectToken);
        //                selectedIndex++;
        //            }
        //            else if (selectedIndex == 1)
        //            {
        //                SetVoiceForAudioLog = 2;
        //                //pISpVoice->SetVoice(pISpObjectToken);
        //            }
        //        }
        //        break;
        //    }
        //    break;

        //  ***********************************************

        case MY_ABOUT_PUSH_BUTTON: 
            pISpVoice->Speak(L"Hello !!! Welcome To My Profile ", SPF_ASYNC, NULL); 
              DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(ABOUT_DIALOG), hEditImageDlg, AboutDlgProc);
              fprintf(gpFile_UserLog, "%02d:%02d:%02d - User Clicked On About Button.\n", localTime.wHour, localTime.wMinute, localTime.wSecond);

        break;
        //case REGISTER_BUTTON:
        //    pISpVoice->Speak(L"Please Enter The User Details", SPF_ASYNC, NULL);
        //    DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(REGISTER_DIALOG), hEditImageDlg, RegisterDlgProc);
        //    fprintf(gpFile_UserLog, "%02d:%02d:%02d - User Clicked On Register Button.\n", localTime.wHour, localTime.wMinute, localTime.wSecond);

     
            InvalidateRect(hEditImageDlg, NULL, TRUE);
            break;
        
        case DESATURATION_PUSHBUTTON:
            bApplyDesaturation = TRUE;
            pISpVoice->Speak(L"Desaturation Effect Apply For Image", SPF_ASYNC, NULL);
            InvalidateRect(GetParent(hEditImageDlg), NULL, TRUE);
            fprintf(gpFile_UserLog, "%02d:%02d:%02d - User Selected Desaturation Effect.\n", localTime.wHour, localTime.wMinute, localTime.wSecond);

            break;
        case DESATURATION_RESET:
            bApplyOriginalColor = TRUE;
            pISpVoice->Speak(L"Image Reset", SPF_ASYNC, NULL);
            InvalidateRect(GetParent(hEditImageDlg), NULL, TRUE);
            fprintf(gpFile_UserLog, "%02d:%02d:%02d - User Clicked On Desaturation Reset Button.\n", localTime.wHour, localTime.wMinute, localTime.wSecond);

            break;
        case SEPIA_PUSHBUTTON:
            bApplySepia = TRUE;
            pISpVoice->Speak(L"Sepia Effect Apply For Image", SPF_ASYNC, NULL);
            InvalidateRect(GetParent(hEditImageDlg), NULL, TRUE);
            fprintf(gpFile_UserLog, "%02d:%02d:%02d - User Selected Sepia Effect.\n", localTime.wHour, localTime.wMinute, localTime.wSecond);

            break;
        case SEPIA_RESET:
            bApplyOriginalColor = TRUE;
            pISpVoice->Speak(L"Image Reset", SPF_ASYNC, NULL);
            InvalidateRect(GetParent(hEditImageDlg), NULL, TRUE);
            fprintf(gpFile_UserLog, "%02d:%02d:%02d - User Clicked On Sepia Reset Button.\n", localTime.wHour, localTime.wMinute, localTime.wSecond);

            break;
        case COLOR_INVERSION_PUSHBUTTON:
            bApplyColorInversion = TRUE;
            pISpVoice->Speak(L"Color Inversion Effect Apply For Image", SPF_ASYNC, NULL);
            InvalidateRect(GetParent(hEditImageDlg), NULL, TRUE);
            fprintf(gpFile_UserLog, "%02d:%02d:%02d - User Apply Color Inversion Effect.\n", localTime.wHour, localTime.wMinute, localTime.wSecond);

            break;
        case COLOR_INVERSION_RESET:
            bApplyOriginalColor = TRUE;
            pISpVoice->Speak(L"Image Reset", SPF_ASYNC, NULL);
            InvalidateRect(GetParent(hEditImageDlg), NULL, TRUE);
            fprintf(gpFile_UserLog, "%02d:%02d:%02d - User Clicked On Color Inversion Reset Button.\n", localTime.wHour, localTime.wMinute, localTime.wSecond);

            break;
        case RD_IMAGE_PROCESSING:
            bColorPickerOn = FALSE;
            SendDlgItemMessage(hEditImageDlg, RD_IMAGE_PROCESSING, BM_SETCHECK, 1, 0); 
            SendDlgItemMessage(hEditImageDlg, RD_COLOR_PICKER, BM_SETCHECK, 0, 0);
            EnableWindow(GetDlgItem(hEditImageDlg, COLOR_PICKER), FALSE);
            EnableWindow(GetDlgItem(hEditImageDlg, R_TEXT_BOX), FALSE);
            EnableWindow(GetDlgItem(hEditImageDlg, G_TEXT_BOX), FALSE);
            EnableWindow(GetDlgItem(hEditImageDlg, B_TEXT_BOX), FALSE);
            EnableWindow(GetDlgItem(hEditImageDlg, COLOR_PICKER_BOX), FALSE);
            EnableWindow(GetDlgItem(hEditImageDlg, PICKED_COLOR), FALSE);
            EnableWindow(GetDlgItem(hEditImageDlg, RED_VALUE), FALSE);
            EnableWindow(GetDlgItem(hEditImageDlg, GREEN_VALUE), FALSE);
            EnableWindow(GetDlgItem(hEditImageDlg, BLUE_VALUE), FALSE);
            EnableWindow(GetDlgItem(hEditImageDlg, COLOR_PICKER_BOX), FALSE);
            EnableWindow(GetDlgItem(hEditImageDlg, EXPORT_RGB_CHECK), FALSE);
            EnableWindow(GetDlgItem(hEditImageDlg, NORMALISE_RGB_CHECK), FALSE);
           
            EnableWindow(GetDlgItem(hEditImageDlg, ID_IMAGE_SETS), TRUE);
            EnableWindow(GetDlgItem(hEditImageDlg, MY_IMAGE_EDITOR), TRUE);
            EnableWindow(GetDlgItem(hEditImageDlg, IMAGE_DESATURATION), TRUE);
            EnableWindow(GetDlgItem(hEditImageDlg, IMAGE_COLOR_INVERSION), TRUE);
            EnableWindow(GetDlgItem(hEditImageDlg, IMAGE_SEPIA), TRUE);
            EnableWindow(GetDlgItem(hEditImageDlg, DESATURATION_PUSHBUTTON), TRUE);
            EnableWindow(GetDlgItem(hEditImageDlg, DESATURATION_RESET), TRUE);
            EnableWindow(GetDlgItem(hEditImageDlg, SEPIA_PUSHBUTTON), TRUE);
            EnableWindow(GetDlgItem(hEditImageDlg, SEPIA_RESET), TRUE);
            EnableWindow(GetDlgItem(hEditImageDlg, COLOR_INVERSION_PUSHBUTTON), TRUE);
            EnableWindow(GetDlgItem(hEditImageDlg, COLOR_INVERSION_RESET), TRUE);

            pISpVoice->Speak(L"Image Effect Option Is Selected", SPF_ASYNC, NULL); 

                sprintf(str, TEXT("%02d:%02d:%02d - Image Processing Radio Button is Selected.\n"), localTime.wHour, localTime.wMinute, localTime.wSecond);
                fprintf(gpFile_UserLog, str);

            break;
        case RD_COLOR_PICKER:
            bColorPickerOn = TRUE;

            pISpVoice->Speak(L"Color Picker Option Is Selected", SPF_ASYNC, NULL);

            SendDlgItemMessage(hEditImageDlg, RD_COLOR_PICKER, BM_SETCHECK, 1, 0);
            SendDlgItemMessage(hEditImageDlg, RD_IMAGE_PROCESSING, BM_SETCHECK, 0, 0);
            EnableWindow(GetDlgItem(hEditImageDlg, COLOR_PICKER), TRUE);
            EnableWindow(GetDlgItem(hEditImageDlg, R_TEXT_BOX), TRUE);
            EnableWindow(GetDlgItem(hEditImageDlg, G_TEXT_BOX), TRUE);
            EnableWindow(GetDlgItem(hEditImageDlg, B_TEXT_BOX), TRUE);
            EnableWindow(GetDlgItem(hEditImageDlg, COLOR_PICKER_BOX), TRUE);
            EnableWindow(GetDlgItem(hEditImageDlg, PICKED_COLOR), TRUE);
            EnableWindow(GetDlgItem(hEditImageDlg, RED_VALUE), TRUE);
            EnableWindow(GetDlgItem(hEditImageDlg, GREEN_VALUE), TRUE);
            EnableWindow(GetDlgItem(hEditImageDlg, BLUE_VALUE), TRUE);
            EnableWindow(GetDlgItem(hEditImageDlg, COLOR_PICKER_BOX), TRUE);
            EnableWindow(GetDlgItem(hEditImageDlg, EXPORT_RGB_CHECK), TRUE);
            EnableWindow(GetDlgItem(hEditImageDlg, NORMALISE_RGB_CHECK), TRUE);
           
            EnableWindow(GetDlgItem(hEditImageDlg, DESATURATION_PUSHBUTTON), FALSE);
            EnableWindow(GetDlgItem(hEditImageDlg, DESATURATION_RESET), FALSE);
            EnableWindow(GetDlgItem(hEditImageDlg, SEPIA_PUSHBUTTON), FALSE);
            EnableWindow(GetDlgItem(hEditImageDlg, SEPIA_RESET), FALSE);
            EnableWindow(GetDlgItem(hEditImageDlg, COLOR_INVERSION_PUSHBUTTON), FALSE);
            EnableWindow(GetDlgItem(hEditImageDlg, COLOR_INVERSION_RESET), FALSE);
            EnableWindow(GetDlgItem(hEditImageDlg, ID_IMAGE_SETS), FALSE);
            EnableWindow(GetDlgItem(hEditImageDlg, MY_IMAGE_EDITOR), FALSE);
            EnableWindow(GetDlgItem(hEditImageDlg, IMAGE_DESATURATION), FALSE);
            EnableWindow(GetDlgItem(hEditImageDlg, IMAGE_COLOR_INVERSION), FALSE);
            EnableWindow(GetDlgItem(hEditImageDlg, IMAGE_SEPIA), FALSE);

            sprintf(str, TEXT("%02d:%02d:%02d - Color Picker Radio Button is Selected.\n"), localTime.wHour, localTime.wMinute, localTime.wSecond);
                fprintf(gpFile_UserLog, str);
            break;

        case EXPORT_RGB_CHECK:
            pISpVoice->Speak(L"User Export Picked Colors Log File", SPF_ASYNC, NULL);
            if(bExportPickedColors)
            {
                SendDlgItemMessage(hEditImageDlg, EXPORT_RGB_CHECK, BM_SETCHECK, 0, 0);
                bExportPickedColors = FALSE;
                fprintf(gpFile_UserLog, "%02d:%02d:%02d - User Clicked On Export Picked Colors .\n", localTime.wHour, localTime.wMinute, localTime.wSecond);

            }
            else
            {
                SendDlgItemMessage(hEditImageDlg, EXPORT_RGB_CHECK, BM_SETCHECK, 1, 0);
                bExportPickedColors = TRUE;
            }
            break;

        case NORMALISE_RGB_CHECK: 
            pISpVoice->Speak(L"User Export Normalised Picked Colors Log File", SPF_ASYNC, NULL);
            if (bNormalisedPickedColors)
            {
                SendDlgItemMessage(hEditImageDlg, NORMALISE_RGB_CHECK, BM_SETCHECK, 0, 0);
                bNormalisedPickedColors = FALSE;
                fprintf(gpFile_UserLog, "%02d:%02d:%02d - User Clicked On Export Normalized Picked Colors.\n", localTime.wHour, localTime.wMinute, localTime.wSecond);

            }
            else
            {
                SendDlgItemMessage(hEditImageDlg, NORMALISE_RGB_CHECK, BM_SETCHECK, 1, 0);
                bNormalisedPickedColors = TRUE;
            }
            break;
      
        default:
            break;

        }
        return(TRUE);

      case WM_PAINT:

          if (gpFile_UserLog)
          {
              EnableWindow(GetDlgItem(hEditImageDlg, REGISTER_BUTTON), FALSE);
              SendDlgItemMessage(hEditImageDlg, RD_IMAGE_PROCESSING, BM_SETCHECK, 1, 0);
              EnableWindow(GetDlgItem(hEditImageDlg, RD_IMAGE_PROCESSING), TRUE);
              EnableWindow(GetDlgItem(hEditImageDlg, RD_COLOR_PICKER), TRUE);
              EnableWindow(GetDlgItem(hEditImageDlg, DESATURATION_PUSHBUTTON), TRUE);
              EnableWindow(GetDlgItem(hEditImageDlg, DESATURATION_RESET), TRUE);
              EnableWindow(GetDlgItem(hEditImageDlg, SEPIA_PUSHBUTTON), TRUE);
              EnableWindow(GetDlgItem(hEditImageDlg, SEPIA_RESET), TRUE);
              EnableWindow(GetDlgItem(hEditImageDlg, COLOR_INVERSION_PUSHBUTTON), TRUE);
              EnableWindow(GetDlgItem(hEditImageDlg, COLOR_INVERSION_RESET), TRUE);
              EnableWindow(GetDlgItem(hEditImageDlg, ID_IMAGE_SETS), TRUE);
              EnableWindow(GetDlgItem(hEditImageDlg, MY_IMAGE_EDITOR), TRUE);
              EnableWindow(GetDlgItem(hEditImageDlg, IMAGE_DESATURATION), TRUE);
              EnableWindow(GetDlgItem(hEditImageDlg, IMAGE_COLOR_INVERSION), TRUE);
              EnableWindow(GetDlgItem(hEditImageDlg, IMAGE_SEPIA), TRUE);
              EnableWindow(GetDlgItem(hEditImageDlg, REGISTER_DIALOG), FALSE);
              EnableMenuItem(GetMenu(GetParent(hEditImageDlg)), IDM_OPEN_FILE, MF_ENABLED); 
          }
      
      break;

      case WM_USER + 1:
          if (wParam == 1)
          {
              // MessageBox(NULL, TEXT("IN"), TEXT("Error"), MB_OK);

              colorPickerBox = (COLORREF)lParam;

              if (hBrushColorPicker)
              {
                  DeleteObject(hBrushColorPicker);
              }
              // MessageBox(NULL, TEXT("out"), TEXT("Error"), MB_OK);
              hBrushColorPicker = CreateSolidBrush(colorPickerBox);
              InvalidateRect(GetDlgItem(hEditImageDlg, ID_EDIT_COLOR), NULL, TRUE);
          }
          break;
  
      case WM_CLOSE:
          if (hBrushColorPicker) 
          {
              DeleteObject(hBrushColorPicker); 
          }

         DestroyWindow(hEditImageDlg);
         hwndModeless = 0; 
         return(TRUE);

    case WM_CTLCOLORDLG:
        hBrushDlgBk = CreateSolidBrush(RGB(135, 206, 235));
        return (INT_PTR)hBrushDlgBk;

    case WM_CTLCOLORSTATIC:
        hdcStatic = (HDC)wParam;
        SetTextColor(hdcStatic, RGB(0,0,0));

        if (lParam == (LPARAM)GetDlgItem(hEditImageDlg, ID_EDIT_COLOR))
        {
            SetBkColor(hdcStatic, colorPickerBox);  
            return (INT_PTR)hBrushColorPicker;
        }
        else
        {
            SetBkColor(hdcStatic, RGB(135, 206, 235));
            hBrushTextBk = CreateSolidBrush(RGB(135, 206, 235));
            return (INT_PTR)hBrushTextBk;
        }
    }
    return(FALSE);
}

INT_PTR CALLBACK RegisterDlgProc(HWND hRegistrationDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    HBRUSH hBrushDlgBk = NULL;
    HWND   hwndFirstName = NULL;
    HWND   hwndMiddleName = NULL;
    HWND   hwndLastName = NULL;
    HBRUSH hBrushFirstNameTextBk = NULL;
    HBRUSH hBrushTextBk = NULL;
    HBRUSH hBrushMiddleNameTextBk = NULL;
    HBRUSH hBrushLastNameTextBk = NULL;
    HDC hdc = NULL;
    HDC hdcFirstNameStatic = NULL;
    HDC hdcMiddleNameStatic = NULL;
    HDC hdcLastNameStatic = NULL;
    HDC hdcStatic = NULL;
    int    cTxtLen = 0;
    //static LPSTR  lpszFirstName = NULL;
    //static LPSTR  lpszMiddleName = NULL;
    //static LPSTR  lpszLastName = NULL;
    //TCHAR  strFirstName[256]  = {'\0'};
    //TCHAR  strMiddleName[256] = {'\0'};
    //TCHAR  strLastName[256]   = {'\0'};
    //TCHAR str[256] = { '\0' };
    
    switch (iMsg)
    {
    case WM_INITDIALOG:
        //Set Focus in name
        SetFocus(GetDlgItem(hRegistrationDlg, 1));
        EnableWindow(GetDlgItem(hRegistrationDlg, IDOK), FALSE);
        EnableMenuItem(GetMenu(GetParent(hRegistrationDlg)), IDM_OPEN_FILE, MF_ENABLED); 
        EnableMenuItem(GetMenu(GetParent(hRegistrationDlg)), IDM_EDIT_IMAGE, MF_ENABLED); 

        return(TRUE);
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
        {
            GetLocalTime(&time);
            TCHAR strUserName[512] = { '\0' };
          
            wsprintf(strUserName, "%s %s", lpszFirstName, lpszLastName);
            sprintf(strUserLogName, strUserName);


            if (CreateUserLogFile(strUserName))
            {
                //  ******* User Log File   ***************
                sprintf(str, TEXT("User Name is : %s %s %s \n"), lpszFirstName, lpszMiddleName, lpszLastName);
                fprintf(gpFile_UserLog, str);

                sprintf(str, TEXT("User Registration Date :- %0.2d / %0.2d / %0.2d\n"), time.wDay, time.wMonth, time.wYear);
                fprintf(gpFile_UserLog, str);

                sprintf(str, TEXT("User Registration Time :- %0.2d Hours  %0.2d Minutes  %0.2d Seconds\n"), time.wHour, time.wMinute, time.wSecond);
                fprintf(gpFile_UserLog, str);

                sprintf(str, TEXT("\n"));
                fprintf(gpFile_UserLog, str);

                sprintf(str, TEXT(".......User Activities For User Log are ..... \n\n"));
                fprintf(gpFile_UserLog, str);

                sprintf(str, TEXT("%02d:%02d:%02d - User Registered Successfully!\n"), time.wHour, time.wMinute, time.wSecond);
                fprintf(gpFile_UserLog, str);

                pISpVoice->Speak(L"Registration Done Successfully", SPF_ASYNC, NULL);
                MessageBox(NULL, "Registration Done Successfully!\n", TEXT("SUCSSES"), MB_OKCANCEL);

                pISpVoice->Speak(L"Click On File menu to Select an Image for Processing", SPF_ASYNC, NULL);

                InvalidateRect(GetParent(hRegistrationDlg), NULL, TRUE);
                EndDialog(hRegistrationDlg, IDCANCEL);

            }

            if (CreatePickedColorLogFile(strUserName))
            {
                //  *************   Picked Color Log File   *************

                sprintf(str, TEXT("User Name is : %s %s %s \n"), lpszFirstName, lpszMiddleName, lpszLastName);
                fprintf(gpFile_ExportPickedColors, str);

                sprintf(str, TEXT("User Registration Date :- %0.2d - %0.2d - %0.2d\n"), time.wDay, time.wMonth, time.wYear);
                fprintf(gpFile_ExportPickedColors, str);

                sprintf(str, TEXT("User Registration Time :- %0.2d Hours  %0.2d Minutes  %0.2d Seconds\n"), time.wHour, time.wMinute, time.wSecond);
                fprintf(gpFile_ExportPickedColors, str);

                sprintf(str, TEXT("\n"));
                fprintf(gpFile_ExportPickedColors, str);

                sprintf(str, TEXT("...... User Activities for Picked Color are ......\n\n"));
                fprintf(gpFile_ExportPickedColors, str);
            }

            if (CreateNormalizedColorLogFile(strUserName))
            {
                //  *************   Normalized Picked Color Log File    *****************

                sprintf(str, TEXT("User Name is : %s %s %s \n"), lpszFirstName, lpszMiddleName, lpszLastName);
                fprintf(gpFile_NormalisedPickedColors, str);

                sprintf(str, TEXT("User Registration Date :- %0.2d - %0.2d - %0.2d\n"), time.wDay, time.wMonth, time.wYear);
                fprintf(gpFile_NormalisedPickedColors, str);

                sprintf(str, TEXT("User Registration Time :- %0.2d Hours  %0.2d Minutes  %0.2d Seconds\n"), time.wHour, time.wMinute, time.wSecond);
                fprintf(gpFile_NormalisedPickedColors, str);

                sprintf(str, TEXT("\n"));
                fprintf(gpFile_NormalisedPickedColors, str);

                sprintf(str, TEXT(".... User Activities For Normalized Picked Color are  ..\n\n"));
                fprintf(gpFile_NormalisedPickedColors, str);
            }

            else
            {
                DestroyWindow(GetParent(GetParent(hRegistrationDlg)));
                //PostQuitMessage(0);
            }
        }
        break;

        case IDCANCEL:
            if (gpFile_UserLog)
            {
                sprintf(str, TEXT("%02d:%02d:%02d - Closing Dialog Box.\n"), time.wHour, time.wMinute, time.wSecond);
                fprintf(gpFile_UserLog, str);
            }
                
            EndDialog(hRegistrationDlg, IDCANCEL);
            break;

        default:
            break;
        }
        switch (HIWORD(wParam))
        {
        case EN_KILLFOCUS:
            //*** First Name ***
           // SetDlgItemInt(hRegistrationDlg, FIRST_NAME, 123, FALSE);// Example code for SetDlgItem
            hwndFirstName = GetDlgItem(hRegistrationDlg, FIRST_NAME);
            cTxtLen = GetWindowTextLength(hwndFirstName);

            lpszFirstName = (PSTR)VirtualAlloc((LPVOID)NULL, (DWORD)(cTxtLen + 1), MEM_COMMIT, PAGE_READWRITE);
            GetWindowText(hwndFirstName, lpszFirstName, (cTxtLen + 1));
            wsprintf(strFirstName, "%s", lpszFirstName);

            //*** Middle Name ***
            hwndMiddleName = GetDlgItem(hRegistrationDlg, MIDDLE_NAME);
            cTxtLen = GetWindowTextLength(hwndMiddleName);

            lpszMiddleName = (PSTR)VirtualAlloc((LPVOID)NULL, (DWORD)(cTxtLen + 1), MEM_COMMIT, PAGE_READWRITE);
            GetWindowText(hwndMiddleName, lpszMiddleName, (cTxtLen + 1));
            wsprintf(strMiddleName, "%s", lpszMiddleName);
           
            //*** Last Name ***
            hwndLastName = GetDlgItem(hRegistrationDlg, LAST_NAME);
            cTxtLen = GetWindowTextLength(hwndLastName);

            lpszLastName = (PSTR)VirtualAlloc((LPVOID)NULL, (DWORD)(cTxtLen + 1), MEM_COMMIT, PAGE_READWRITE);
            GetWindowText(hwndLastName, lpszLastName, (cTxtLen + 1));
            wsprintf(strLastName, "%s", lpszLastName);
           
            if ((strlen(strFirstName) != 0) && (strlen(strMiddleName) != 0) && (strlen(strLastName) != 0))//NULL CHECK
                bNotNullOrEmptyString = TRUE;
            else
                bNotNullOrEmptyString = FALSE;
           
            bCatchFirstName = ValidateString(strFirstName);// Validate String
            SendDlgItemMessage(hRegistrationDlg, F_NAME_STATIC, WM_ENABLE, TRUE, 0l);
           
            bCatchMiddleName = ValidateString(strMiddleName);// Validate String
            SendDlgItemMessage(hRegistrationDlg, M_NAME_STATIC, WM_ENABLE, TRUE, 0l);
            
            bCatchLastName = ValidateString(strLastName);// Validate String
            SendDlgItemMessage(hRegistrationDlg, L_NAME_STATIC, WM_ENABLE, TRUE, 0l);

            if (bNotNullOrEmptyString)
            {
                if((bCatchFirstName) && (bCatchMiddleName) && (bCatchLastName))
                 EnableWindow(GetDlgItem(hRegistrationDlg, IDOK), TRUE);
                else 
                    EnableWindow(GetDlgItem(hRegistrationDlg, IDOK), FALSE);
            } 
            else if(!bNotNullOrEmptyString)
                EnableWindow(GetDlgItem(hRegistrationDlg, IDOK), FALSE);
           
            break;
        }
        return(TRUE);
    case WM_CTLCOLORDLG:
        hBrushDlgBk = CreateSolidBrush(RGB(135, 206, 235));
        return (INT_PTR)hBrushDlgBk;
    case WM_CTLCOLORSTATIC:
        hdcFirstNameStatic  = (HDC)wParam;
        hdcMiddleNameStatic = (HDC)wParam;
        hdcLastNameStatic   = (HDC)wParam;
        hdcStatic = (HDC)wParam;

        if (lParam == (LPARAM)GetDlgItem(hRegistrationDlg, USER_NAME))
        {
            SetTextColor(hdcStatic, RGB(0, 0, 0));
            SetBkColor(hdcStatic, RGB(135, 206, 235));
            hBrushTextBk = CreateSolidBrush(RGB(135, 206, 235));
            return (INT_PTR)hBrushTextBk;
        }
        // ******* First Name Static Label
        if (lParam == (LPARAM)GetDlgItem(hRegistrationDlg, F_NAME_STATIC))
        {
            SetTextColor(hdcFirstNameStatic, RGB(0, 0, 0));
            SetBkColor(hdcFirstNameStatic, RGB(135, 206, 235));
            if (!bCatchFirstName)
            {
                SetTextColor(hdcFirstNameStatic, RGB(255, 0, 0));//RED
                SetBkColor(hdcFirstNameStatic, RGB(135, 206, 235));
            }
            else
            {
                SetTextColor(hdcFirstNameStatic, RGB(0, 0, 0));//BLACK
                SetBkColor(hdcFirstNameStatic, RGB(135, 206, 235));
            }
            hBrushFirstNameTextBk = CreateSolidBrush(RGB(135, 206, 235));
            return (INT_PTR)hBrushFirstNameTextBk;
        }
        // ******* Middle Name Static Label
        if (lParam == (LPARAM)GetDlgItem(hRegistrationDlg, M_NAME_STATIC))
        {
            SetTextColor(hdcMiddleNameStatic, RGB(0, 0, 0));
            SetBkColor(hdcMiddleNameStatic, RGB(135, 206, 235));
            if (!bCatchMiddleName)
            {
                SetTextColor(hdcMiddleNameStatic, RGB(255, 0, 0));//RED
                SetBkColor(hdcMiddleNameStatic, RGB(135, 206, 235));
            }
            else
            {
                SetTextColor(hdcMiddleNameStatic, RGB(0, 0, 0));//BLACK
                SetBkColor(hdcMiddleNameStatic, RGB(135, 206, 235));
            }
            hBrushMiddleNameTextBk = CreateSolidBrush(RGB(135, 206, 235));
            return (INT_PTR)hBrushMiddleNameTextBk;
        }
        // ******* Last Name Static Label
        if (lParam == (LPARAM)GetDlgItem(hRegistrationDlg, L_NAME_STATIC))
        {
            SetTextColor(hdcLastNameStatic, RGB(0, 0, 0));
            SetBkColor(hdcLastNameStatic, RGB(135, 206, 235));
            if (!bCatchLastName)
            {
                SetTextColor(hdcLastNameStatic, RGB(255, 0, 0));//RED
                SetBkColor(hdcLastNameStatic, RGB(135, 206, 235));
            }
            else
            {
                SetTextColor(hdcLastNameStatic, RGB(0, 0, 0));//BLACK
                SetBkColor(hdcLastNameStatic, RGB(135, 206, 235));
            }
            hBrushLastNameTextBk = CreateSolidBrush(RGB(135, 206, 235));
            return (INT_PTR)hBrushLastNameTextBk;
        }
    }
    return(FALSE);
}

INT_PTR CALLBACK AboutDlgProc(HWND hwndAbout, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    HDC hdcStatic = NULL;
    HBRUSH hBrushDlgBk = NULL, hBrushTextBk = NULL;
    switch (iMsg)
    {
    case WM_INITDIALOG:
          if(gpFile_UserLog)
                fprintf(gpFile_UserLog, "%02d:%02d:%02d - User Opened The About Dialog Box.\n", time.wHour, time.wMinute, time.wSecond);
        
        SetFocus(GetDlgItem(hwndAbout, 1));
        return(TRUE);

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            if (gpFile_UserLog)
                fprintf(gpFile_UserLog, "%02d:%02d:%02d - User Closed The About Dialog Box.\n", time.wHour, time.wMinute, time.wSecond);
           
            if (hBrushDlgBk)
            {
                DeleteObject(hBrushDlgBk);
            }

            if (hBrushTextBk)
            {
                DeleteObject(hBrushTextBk);
            }
            if (hdcStatic)
            {
                DeleteDC(hdcStatic);
                hdcStatic = NULL;
            }
            EndDialog(hwndAbout, IDOK);
            break;
        }
        return(TRUE);

    case WM_CLOSE:
        if(gpFile_UserLog)
                fprintf(gpFile_UserLog, "%02d:%02d:%02d - User Closed The About Dialog Box.\n", time.wHour, time.wMinute, time.wSecond);
        if (hBrushDlgBk)
        {
            DeleteObject(hBrushDlgBk);
        }

        if (hBrushTextBk)
        {
            DeleteObject(hBrushTextBk);
        }

        if (hdcStatic)
        {
            DeleteDC(hdcStatic);
            hdcStatic = NULL;
        }

        EndDialog(hwndAbout, WM_CLOSE);
        return(TRUE);

    case WM_CTLCOLORDLG:
        hBrushDlgBk = CreateSolidBrush(RGB(0, 0, 0));
        return (INT_PTR)hBrushDlgBk;

    case WM_CTLCOLORSTATIC:
        hdcStatic = (HDC)wParam;
        SetTextColor(hdcStatic, RGB(0, 255, 0));//Green 
        //SetTextColor(hdcStatic, RGB(255, 19, 240));//Baby Pink
        SetBkColor(hdcStatic, RGB(0, 0, 0));
        hBrushTextBk = CreateSolidBrush(RGB(0, 0, 0));
        return (INT_PTR)hBrushTextBk;
    }
    return(FALSE);
}

BOOL ValidateString(TCHAR* str)
{
    int iCnt = 0;

    for (iCnt = 0; *(str + iCnt) != '\0'; iCnt++)
    {
        if (isalpha(*(str + iCnt)) == 0)
        {
            return(FALSE);
            break;
        }
    }
    return(TRUE);
}



BOOL CreateUserLogFile(TCHAR* strName)
{
    TCHAR str[256] = { '\0' };
    GetLocalTime(&time);

    if (!gpFile_UserLog) 
    {
        sprintf(str, TEXT("%s_UserLog_%02d-%02d-%04d_%02d-%02d-%02d.txt"), strName, time.wDay, time.wMonth, time.wYear, time.wHour, time.wMinute, time.wSecond);
     
        gpFile_UserLog = fopen(str, "w");

        if (!gpFile_UserLog)
        {
            MessageBox(NULL, TEXT("Error In Creating User Log File"), TEXT("FILE HANDLING ERROR"), MB_OKCANCEL | MB_ICONERROR);
            return (FALSE);
        }
    }
    return(TRUE);
}

BOOL CreatePickedColorLogFile(TCHAR* strName)
{
    if (!gpFile_ExportPickedColors)
    {
        TCHAR str[256] = { '\0' };
        GetLocalTime(&time);

        sprintf(str, TEXT("%s_PickedColorLog_%02d-%02d-%04d_%02d-%02d-%02d.txt"), strName, time.wDay, time.wMonth, time.wYear, time.wHour, time.wMinute, time.wSecond);

        gpFile_ExportPickedColors = fopen(str, "w");
        if (!gpFile_ExportPickedColors)
        {
            MessageBox(NULL, TEXT("Error In Creating Picked Color Log File"), TEXT("FILE HANDLING ERROR"), MB_OKCANCEL | MB_ICONERROR);
            return (FALSE);
        }
    }
    return(TRUE);

}

BOOL CreateNormalizedColorLogFile(TCHAR* strName)
{
    if (!gpFile_NormalisedPickedColors)
    {
        TCHAR str[256] = { '\0' };
        GetLocalTime(&time);

        sprintf(str, TEXT("%s_NormalisedColorLog_%02d-%02d-%04d_%02d-%02d-%02d.txt"), strName, time.wDay, time.wMonth, time.wYear, time.wHour, time.wMinute, time.wSecond);
        //sprintf(str, TEXT("%s_NormalisedColorLog_%02d-%02d-%04d_%02d-%02d-%02d.txt"), strName, time.wDay, time.wMonth, time.wYear, time.wHour, time.wMinute, time.wSecond);
        gpFile_NormalisedPickedColors = fopen(str, "w");

        if (!gpFile_NormalisedPickedColors)
        {
            MessageBox(NULL, TEXT("Error In Creating Normalised Picked Color Log File"), TEXT("FILE HANDLING ERROR"), MB_OKCANCEL | MB_ICONERROR);
            return (FALSE);
        }
    }
    return(TRUE);
}

void UnInitializeLogFiles()
{
    GetLocalTime(&time);

    if (gpFile_UserLog)
    {
        fprintf(gpFile_UserLog, "\n %02d:%02d:%02d - User Terminated The Program!\n\n", time.wHour, time.wMinute, time.wSecond);
        fclose(gpFile_UserLog);
        gpFile_UserLog = NULL;
    }

    if (gpFile_ExportPickedColors)
    {
        fclose(gpFile_ExportPickedColors);
        gpFile_ExportPickedColors = NULL;
    }

    if (gpFile_NormalisedPickedColors)
    {
        fclose(gpFile_NormalisedPickedColors);
        gpFile_NormalisedPickedColors = NULL;
    }
}

void SafeInterfaceRelease(void)
{

      //********************************
    if (pISpStream)
    {
        pISpStream->Release();
        pISpStream = NULL;
    }

    if (pISpObjectToken2)
    {
        pISpObjectToken2->Release();
        pISpObjectToken2 = NULL;
    }

    if (pISpObjectToken1)
    {
        pISpObjectToken1->Release();
        pISpObjectToken1 = NULL;
    }

    if (pIEnumSpObjectTokens)
    {
        pIEnumSpObjectTokens->Release();
        pIEnumSpObjectTokens = NULL;
    }

    if (pISpVoice)
    {
        pISpVoice->Release();
        pISpVoice = NULL;
    }
    //  ***************************
    if (pInegative)
    {
        pInegative->Release();
        pInegative = NULL;
    }

    if (pIsepia)
    {
        pIsepia->Release();
        pIsepia = NULL;
    }

    if (pIdesaturation)
    {
        pIdesaturation->Release();
        pIdesaturation = NULL;

    }

}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             