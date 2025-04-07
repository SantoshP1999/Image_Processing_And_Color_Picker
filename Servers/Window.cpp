#include<windows.h>
#include<commdlg.h>
#include<stdio.h>
#include<stdlib.h>
#include<malloc.h>
#include<ctype.h>//isalpha()
#include<string.h>//strlen()
#include<windowsx.h>//For get X Param and Y Param
#include "Window.h"
#include "CommonDialog.h"
#include "CombineEffectsHeaderFile.h"
#define UNICODE
#pragma comment(lib, "comdlg32.lib")

//Global Callback Declaration
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR    CALLBACK DlgIPAndColorPickerProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR    CALLBACK DlgRegisterUserProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR    CALLBACK DlgAboutProc(HWND, UINT, WPARAM, LPARAM);
BOOL    ValidateString(TCHAR*);
BOOL    CreateAllLogFiles(TCHAR*);
//void    CloseAllLogFiles();


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

//Window Width & Height
int WIDTH = GetSystemMetrics(SM_CXSCREEN) / 2;
int HEIGHT = GetSystemMetrics(SM_CYSCREEN) / 2;

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
    //  WS_CLIPSIBLI | WS_CLIPSIBLINGS,
    //Create The Window In Memory
    hwnd = CreateWindow(szClassName,
        TEXT("PixEDIT"),
        WS_MINIMIZEBOX|WS_CAPTION| WS_SYSMENU,
        0,
        0,
       // GetSystemMetrics(SM_CXSCREEN)/2,
        //GetSystemMetrics(SM_CYSCREEN)/2,
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
    static int     cxClient, cyClient;
    HDC            hdc, hdcMem;
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
//code
switch (iMsg)
{
case WM_CREATE:
    
    EnableMenuItem(GetMenu(hwnd), IDM_OPEN_FILE, MF_GRAYED);
    hIns = (HINSTANCE)((LPCREATESTRUCT)lParam)->hInstance;

    hResult = CoCreateInstance(CLSID_DesaturationSepia, NULL, CLSCTX_INPROC_SERVER, IID_IDestauration, (void**)&pIdesaturation);

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

    if (!hBitMapFile)
    {
        SetBkColor(hdc, RGB(0, 0, 0));
        SetTextColor(hdc, RGB(0, 255, 0));//Green 
        TextOut(hdc, 175, 175, "Click On File Menu And Select Open An Image File.", 50);
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

    if (bLeftClickForColorPicker)
    {
        COLORREF pickedPixelColorRGB = GetPixel(hdc, pickedPixelXCoord, pickedPixelYCoord);
        pickedPixelColorR = GetRValue(pickedPixelColorRGB);
        pickedPixelColorG = GetGValue(pickedPixelColorRGB);
        pickedPixelColorB = GetBValue(pickedPixelColorRGB);


        normalizedRed = (float)pickedPixelColorR / 255.0f;
        normalizedGreen = (float)pickedPixelColorG / 255.0f;
        normalizedBlue = (float)pickedPixelColorB / 255.0f;

        SetDlgItemInt(hwndModeless, R_TEXT_BOX, pickedPixelColorR, FALSE);//Setting RED color in text box
        SetDlgItemInt(hwndModeless, G_TEXT_BOX, pickedPixelColorG, FALSE);//Setting GREEN color in text box
        SetDlgItemInt(hwndModeless, B_TEXT_BOX, pickedPixelColorB, FALSE);//Setting BLUE color in text box
        bLeftClickForColorPicker = FALSE;

        if (bExportPickedColors)
        {
             fprintf(gpFile_ExportPickedColors, "User Picked color RGB = (%u, %u, %u)\n", pickedPixelColorR, pickedPixelColorG, pickedPixelColorB);
        }
          

        if (bNormalisedPickedColors)
        {   
            fprintf(gpFile_NormalisedPickedColors, "User Picked color normalised RGB = (%.2f, %.2f, %.2f)\n", normalizedRed, normalizedGreen, normalizedBlue);
        }
           

    }
    DeleteDC(hdcMem);
    EndPaint(hwnd, &ps);
}
    break;

case WM_COMMAND:
    
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
      
        InvalidateRect(hwnd, NULL, TRUE);
        break;

    case IDM_EDIT_IMAGE:
        hwndModeless = CreateDialog(hIns, MAKEINTRESOURCE(TSP_DIALOG), hwnd, DlgIPAndColorPickerProc);
        InvalidateRect(hwnd, NULL, TRUE);
        break;

    case IDM_ABOUT:
        DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(ABOUT_DIALOG), hwnd, DlgAboutProc);
        break;
    
    case IDM_EXIT_APP:
        retVal = MessageBox(NULL,TEXT("Do You Really Wants To Exit?"), TEXT("EXIT APPLICATION WARNING"), MB_YESNO | MB_ICONWARNING);

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
    }
    break;
case WM_CLOSE:

    DestroyWindow(hwnd);
   
    break;
case WM_DESTROY:
    if (hBitMapFile)
        DeleteObject(hBitMapFile);
    
    //CloseAllLogFiles();

    fclose(gpFile_ExportPickedColors);
    gpFile_ExportPickedColors = NULL;

    fclose(gpFile_NormalisedPickedColors);
    gpFile_NormalisedPickedColors = NULL; 

    fclose(gpFile_UserLog);
    gpFile_UserLog = NULL;



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
INT_PTR CALLBACK DlgIPAndColorPickerProc(HWND hEditImageDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
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
    SYSTEMTIME t;
    GetLocalTime(&t);
    int retVal = 0;
    switch (iMsg) 
    {
    case WM_INITDIALOG:
        //Set Focus in name
        SetFocus(GetDlgItem(hEditImageDlg, 1));
        //SendDlgItemMessage(hEditImageDlg, RD_IMAGE_PROCESSING, BM_SETCHECK, 1, 0);
        EnableWindow(GetDlgItem(hEditImageDlg, TSP_IMAGE_EDITOR), FALSE);
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

        return(TRUE);

    case WM_COMMAND:

        switch (LOWORD(wParam))
        {
           
        case IDOK:
               
            if(gpFile_UserLog)
                  fprintf(gpFile_UserLog, "%02d:%02d:%02d - Closing Edit Image and Picked Color Dialog Box.\n",time.wHour,time.wMinute,time.wSecond);
            
            EndDialog(hEditImageDlg, IDOK);   
        break;

        case IDCANCEL:
            retVal = MessageBox(NULL, TEXT("Do You Really Wants To Exit?"), TEXT("EXIT APPLICATION WARNING"), MB_YESNO | MB_ICONWARNING);

            switch (retVal)
            {
            case IDYES:
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
        case TSP_ABOUT_PUSH_BUTTON:
              DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(ABOUT_DIALOG), hEditImageDlg, DlgAboutProc);
        break;
        case REGISTER_BUTTON:
            DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(REGISTER_DIALOG), hEditImageDlg, DlgRegisterUserProc);
     
            InvalidateRect(hEditImageDlg, NULL, TRUE);
            break;
        
        case DESATURATION_PUSHBUTTON:
            bApplyDesaturation = TRUE;
            InvalidateRect(GetParent(hEditImageDlg), NULL, TRUE);
            break;
        case DESATURATION_RESET:
            bApplyOriginalColor = TRUE;
            InvalidateRect(GetParent(hEditImageDlg), NULL, TRUE);
            break;
        case SEPIA_PUSHBUTTON:
            bApplySepia = TRUE;
            InvalidateRect(GetParent(hEditImageDlg), NULL, TRUE);
            break;
        case SEPIA_RESET:
            bApplyOriginalColor = TRUE;
            InvalidateRect(GetParent(hEditImageDlg), NULL, TRUE);
            break;
        case COLOR_INVERSION_PUSHBUTTON:
            bApplyColorInversion = TRUE;
            InvalidateRect(GetParent(hEditImageDlg), NULL, TRUE);
            break;
        case COLOR_INVERSION_RESET:
            bApplyOriginalColor = TRUE;
            InvalidateRect(GetParent(hEditImageDlg), NULL, TRUE);
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
           
            EnableWindow(GetDlgItem(hEditImageDlg, TSP_IMAGE_EDITOR), TRUE);
            EnableWindow(GetDlgItem(hEditImageDlg, IMAGE_DESATURATION), TRUE);
            EnableWindow(GetDlgItem(hEditImageDlg, IMAGE_COLOR_INVERSION), TRUE);
            EnableWindow(GetDlgItem(hEditImageDlg, IMAGE_SEPIA), TRUE);
            EnableWindow(GetDlgItem(hEditImageDlg, DESATURATION_PUSHBUTTON), TRUE);
            EnableWindow(GetDlgItem(hEditImageDlg, DESATURATION_RESET), TRUE);
            EnableWindow(GetDlgItem(hEditImageDlg, SEPIA_PUSHBUTTON), TRUE);
            EnableWindow(GetDlgItem(hEditImageDlg, SEPIA_RESET), TRUE);
            EnableWindow(GetDlgItem(hEditImageDlg, COLOR_INVERSION_PUSHBUTTON), TRUE);
            EnableWindow(GetDlgItem(hEditImageDlg, COLOR_INVERSION_RESET), TRUE);

                sprintf(str, TEXT("%02d:%02d:%02d - Image Processing Radio Button is Selected.\n"), time.wHour, time.wMinute, time.wSecond);
                fprintf(gpFile_UserLog, str);
            break;
        case RD_COLOR_PICKER:
            bColorPickerOn = TRUE;
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
            EnableWindow(GetDlgItem(hEditImageDlg, TSP_IMAGE_EDITOR), FALSE);
            EnableWindow(GetDlgItem(hEditImageDlg, IMAGE_DESATURATION), FALSE);
            EnableWindow(GetDlgItem(hEditImageDlg, IMAGE_COLOR_INVERSION), FALSE);
            EnableWindow(GetDlgItem(hEditImageDlg, IMAGE_SEPIA), FALSE);

            sprintf(str, TEXT("%02d:%02d:%02d - Color Picker Radio Button is Selected.\n"), time.wHour, time.wMinute, time.wSecond);
                fprintf(gpFile_UserLog, str);
            break;

        case EXPORT_RGB_CHECK:
            if(bExportPickedColors)
            {
                SendDlgItemMessage(hEditImageDlg, EXPORT_RGB_CHECK, BM_SETCHECK, 0, 0);
                bExportPickedColors = FALSE;
            }
            else
            {
                SendDlgItemMessage(hEditImageDlg, EXPORT_RGB_CHECK, BM_SETCHECK, 1, 0);
                bExportPickedColors = TRUE;
            }
            break;

        case NORMALISE_RGB_CHECK:  
            if (bNormalisedPickedColors)
            {
                SendDlgItemMessage(hEditImageDlg, NORMALISE_RGB_CHECK, BM_SETCHECK, 0, 0);
                bNormalisedPickedColors = FALSE;
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
              EnableWindow(GetDlgItem(hEditImageDlg, TSP_IMAGE_EDITOR), TRUE);
              EnableWindow(GetDlgItem(hEditImageDlg, IMAGE_DESATURATION), TRUE);
              EnableWindow(GetDlgItem(hEditImageDlg, IMAGE_COLOR_INVERSION), TRUE);
              EnableWindow(GetDlgItem(hEditImageDlg, IMAGE_SEPIA), TRUE);
              EnableWindow(GetDlgItem(hEditImageDlg, REGISTER_DIALOG), FALSE);
              EnableMenuItem(GetMenu(GetParent(hEditImageDlg)), IDM_OPEN_FILE, MF_ENABLED);
          }
      
      break;
  
      case WM_CLOSE:
         DestroyWindow(hEditImageDlg);
         hwndModeless = 0;
         return(TRUE);

    case WM_CTLCOLORDLG:
        hBrushDlgBk = CreateSolidBrush(RGB(135, 206, 235));
        return (INT_PTR)hBrushDlgBk;

    case WM_CTLCOLORSTATIC:
        hdcStatic = (HDC)wParam;
        SetTextColor(hdcStatic, RGB(0, 0, 0));
        SetBkColor(hdcStatic, RGB(135, 206, 235));
        hBrushTextBk = CreateSolidBrush(RGB(135, 206, 235));
        return (INT_PTR)hBrushTextBk;
    }
    return(FALSE);
}

INT_PTR CALLBACK DlgRegisterUserProc(HWND hRegistrationDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
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
    static LPSTR  lpszFirstName = NULL;
    static LPSTR  lpszMiddleName = NULL;
    static LPSTR  lpszLastName = NULL;
    TCHAR  strFirstName[256]  = {'\0'};
    TCHAR  strMiddleName[256] = {'\0'};
    TCHAR  strLastName[256]   = {'\0'};
    TCHAR str[256] = { '\0' };
    
    switch (iMsg)
    {
    case WM_INITDIALOG:
        //Set Focus in name
        SetFocus(GetDlgItem(hRegistrationDlg, 1));
        EnableWindow(GetDlgItem(hRegistrationDlg, IDOK), FALSE);

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

            if (CreateAllLogFiles(strUserName))
            {
                //  ******* User Log File   ***************
                sprintf(str, TEXT("User Name is : %s %s %s \n"), lpszFirstName, lpszMiddleName, lpszLastName);
                fprintf(gpFile_UserLog, str); 

                sprintf(str, TEXT("User Registration Date :- %0.2d / %0.2d / %0.2d\n"), time.wDay, time.wMonth, time.wYear);
                fprintf(gpFile_UserLog, str); 

                sprintf(str,TEXT("User Registration Time :- %0.2d Hours  %0.2d Minutes  %0.2d Seconds\n"), time.wHour, time.wMinute, time.wSecond);
                fprintf(gpFile_UserLog, str);

                sprintf(str, TEXT("\n")); 
                fprintf(gpFile_UserLog, str); 

                sprintf(str, TEXT("........................User Activities For User Log are ...................: \n\n"));
                fprintf(gpFile_UserLog, str); 

                //  *************   Picked Color Log File   *************

                sprintf(str, TEXT("User Name is : %s %s %s \n"), lpszFirstName, lpszMiddleName, lpszLastName);
                fprintf(gpFile_ExportPickedColors, str);

                sprintf(str, TEXT("User Registration Date :- %0.2d - %0.2d - %0.2d\n"), time.wDay, time.wMonth, time.wYear);
                fprintf(gpFile_ExportPickedColors, str);

                sprintf(str, TEXT("User Registration Time :- %0.2d Hours  %0.2d Minutes  %0.2d Seconds\n"), time.wHour, time.wMinute, time.wSecond);
                fprintf(gpFile_ExportPickedColors, str);

                sprintf(str, TEXT("\n"));
                fprintf(gpFile_ExportPickedColors, str);

                sprintf(str, TEXT("....................... User Activities for Picked Color are ...................: \n\n"));
                fprintf(gpFile_ExportPickedColors, str);

                //  *************   Normalized Picked Color Log File    *****************

                sprintf(str, TEXT("User Name is : %s %s %s \n"), lpszFirstName, lpszMiddleName, lpszLastName);
                fprintf(gpFile_NormalisedPickedColors, str);

                sprintf(str, TEXT("User Registration Date :- %0.2d - %0.2d - %0.2d\n"), time.wDay, time.wMonth, time.wYear);
                fprintf(gpFile_NormalisedPickedColors, str);

                sprintf(str, TEXT("User Registration Time :- %0.2d Hours  %0.2d Minutes  %0.2d Seconds\n"), time.wHour, time.wMinute, time.wSecond);
                fprintf(gpFile_NormalisedPickedColors, str);

                sprintf(str, TEXT("\n"));
                fprintf(gpFile_NormalisedPickedColors, str);

                sprintf(str, TEXT(".......... User Activities For Normalized Picked Color are  .......: \n\n"));
                fprintf(gpFile_NormalisedPickedColors, str);

                ////

                sprintf(str, TEXT("%02d:%02d:%02d - User Registered Successfully!\n"), time.wHour, time.wMinute, time.wSecond);
                fprintf(gpFile_UserLog, str);
                MessageBox(NULL, "User Registered Successfully!\n", TEXT("SUCSSES"), MB_OKCANCEL);
                InvalidateRect(GetParent(hRegistrationDlg), NULL, TRUE);
                EndDialog(hRegistrationDlg, IDCANCEL);
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

INT_PTR CALLBACK DlgAboutProc(HWND hwndAbout, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    HDC hdcStatic = NULL;
    HBRUSH hBrushDlgBk = NULL, hBrushTextBk = NULL;
    switch (iMsg)
    {
    case WM_INITDIALOG:
          if(gpFile_UserLog)
                fprintf(gpFile_UserLog, "%02d:%02d:%02d - About Dialog Box Opened.\n", time.wHour, time.wMinute, time.wSecond);
        
        SetFocus(GetDlgItem(hwndAbout, 1));
        return(TRUE);

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            if (gpFile_UserLog)
                fprintf(gpFile_UserLog, "%02d:%02d:%02d - About Dialog Box Closed.\n", time.wHour, time.wMinute, time.wSecond);
           
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
                fprintf(gpFile_UserLog, "%02d:%02d:%02d - About Dialog Box Closed.\n", time.wHour, time.wMinute, time.wSecond);
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

BOOL CreateAllLogFiles(TCHAR* strName)
{
    TCHAR str[256] = { '\0' };
    GetLocalTime(&time);

    if (!gpFile_UserLog)
    {
        sprintf(str, TEXT("%s_UserLog_%02d%02d%04d_%02d%02d%02d.txt"), strName, time.wDay, time.wMonth, time.wYear, time.wHour, time.wMinute, time.wSecond);
        gpFile_UserLog = fopen(str, "w");

        if (!gpFile_UserLog)
        {
            MessageBox(NULL, TEXT("Error In Creating User Log File"), TEXT("FILE HANDLING ERROR"), MB_OKCANCEL | MB_ICONERROR);
            return (FALSE);
        }
    }

    if (!gpFile_ExportPickedColors)
    {
        sprintf(str, TEXT("%s_PickedColorLog_%02d%02d%04d_%02d%02d%02d.txt"), strName, time.wDay, time.wMonth, time.wYear, time.wHour, time.wMinute, time.wSecond);

        gpFile_ExportPickedColors = fopen(str, "w");
        if (!gpFile_ExportPickedColors)
        {
            MessageBox(NULL, TEXT("Error In Creating Picked Color Log File"), TEXT("FILE HANDLING ERROR"), MB_OKCANCEL | MB_ICONERROR);
            return (FALSE);
        }
    }
    if (!gpFile_NormalisedPickedColors)
    {
        sprintf(str, TEXT("%s_NormalisedColorLog_%02d%02d%04d_%02d%02d%02d.txt"), strName, time.wDay, time.wMonth, time.wYear, time.wHour,
            time.wMinute, time.wSecond);
        gpFile_NormalisedPickedColors = fopen(str, "w");

        if (!gpFile_NormalisedPickedColors)
        {
            MessageBox(NULL, TEXT("Error In Creating Normalised Picked Color Log File"), TEXT("FILE HANDLING ERROR"), MB_OKCANCEL | MB_ICONERROR);
            return (FALSE);
        }
    }
    return (TRUE);
}

//void CloseAllLogFiles()
//{
//    GetLocalTime(&time);
//    if (gpFile_UserLog)
//    {
//        fprintf(gpFile_UserLog, "%02d:%02d:%02d - Program Terminated Successfully!", time.wHour, time.wMinute, time.wSecond);
//        fclose(gpFile_UserLog);
//        gpFile_UserLog = NULL;
//    }
//
//    if (gpFile_ExportPickedColors)
//    {
//        fclose(gpFile_ExportPickedColors);
//        gpFile_ExportPickedColors = NULL;
//    }
//
//    if (gpFile_NormalisedPickedColors)
//    {
//        fclose(gpFile_NormalisedPickedColors);
//        gpFile_NormalisedPickedColors = NULL;
//    }
//}

void SafeInterfaceRelease(void)
{
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

    if (gpFile_UserLog)
    {
        fclose(gpFile_UserLog);
        gpFile_UserLog = NULL;
    }
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        