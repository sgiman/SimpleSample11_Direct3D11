//***************************************************************************************
// File: SimpleSample11.cpp
//
// Starting point for new Direct3D 11 samples.  For a more basic starting point, 
// use the EmptyProject11 sample instead.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
// Visual Studio 2010
// Microsoft DirectX SDK (June 2010)
//--------------------------------------------------------------------------------------
// Modified by sgiman, 2022, jul
//***************************************************************************************
#include "DXUT.h"
#include "DXUTgui.h"
#include "DXUTmisc.h"
#include "DXUTCamera.h"
#include "DXUTSettingsDlg.h"
#include "SDKmisc.h"
#include "SDKmesh.h"
#include "resource.h"

//#define DEBUG_VS   // Раскомментируйте эту строку для отладки вершинных шейдеров D3D9.
//#define DEBUG_PS   // Раскомментируйте эту строку для отладки пиксельных шейдеров D3D9.

//--------------------------------------------------------------------------------------
// Глобальные переменные
//--------------------------------------------------------------------------------------
CModelViewerCamera          g_Camera;               	// камера просмотра модели
CDXUTDialogResourceManager  g_DialogResourceManager; 	// менеджер общих ресурсов диалогов
CD3DSettingsDlg             g_SettingsDlg;          	// диалоговое окно настроек устройства
CDXUTTextHelper*            g_pTxtHelper = NULL;
CDXUTDialog                 g_HUD;                  	// диалог для стандартных элементов управления
CDXUTDialog                 g_SampleUI;             	// диалоговое окно для конкретных элементов управления образца

// Ресурсы Direct3D 9
extern ID3DXFont*           g_pFont9;
extern ID3DXSprite*         g_pSprite9;

// Ресурсы Direct3D 11
ID3D11VertexShader*         g_pVertexShader11 = NULL;
ID3D11PixelShader*          g_pPixelShader11 = NULL;
ID3D11InputLayout*          g_pLayout11 = NULL;
ID3D11SamplerState*         g_pSamLinear = NULL;

//--------------------------------------------------------------------------------------
// Постоянные буферы
//--------------------------------------------------------------------------------------
#pragma pack(push,1)
struct CB_VS_PER_OBJECT
{
    D3DXMATRIX  m_mWorldViewProjection;
    D3DXMATRIX  m_mWorld;
    D3DXVECTOR4 m_MaterialAmbientColor;
    D3DXVECTOR4 m_MaterialDiffuseColor;
};

struct CB_VS_PER_FRAME
{
    D3DXVECTOR3 m_vLightDir;
    float       m_fTime;
    D3DXVECTOR4 m_LightDiffuse;
};
#pragma pack(pop)

ID3D11Buffer*                       g_pcbVSPerObject11 = NULL;
ID3D11Buffer*                       g_pcbVSPerFrame11 = NULL;

//--------------------------------------------------------------------------------------
// UI control IDs
// Идентификаторы элементов управления пользовательского интерфейса
//--------------------------------------------------------------------------------------
#define IDC_TOGGLEFULLSCREEN    1
#define IDC_TOGGLEREF           2
#define IDC_CHANGEDEVICE        3


//--------------------------------------------------------------------------------------
// Форвардные объявления
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
                          void* pUserContext );
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext );
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext );
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext );
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext );

extern bool CALLBACK IsD3D9DeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat,
                                             bool bWindowed, void* pUserContext );
extern HRESULT CALLBACK OnD3D9CreateDevice( IDirect3DDevice9* pd3dDevice,
                                            const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
extern HRESULT CALLBACK OnD3D9ResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
                                           void* pUserContext );
extern void CALLBACK OnD3D9FrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime,
                                        void* pUserContext );
extern void CALLBACK OnD3D9LostDevice( void* pUserContext );
extern void CALLBACK OnD3D9DestroyDevice( void* pUserContext );

bool CALLBACK IsD3D11DeviceAcceptable( const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo,
                                       DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext );
HRESULT CALLBACK OnD3D11CreateDevice( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
                                     void* pUserContext );
HRESULT CALLBACK OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                                         const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
void CALLBACK OnD3D11ReleasingSwapChain( void* pUserContext );
void CALLBACK OnD3D11DestroyDevice( void* pUserContext );
void CALLBACK OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime,
                                 float fElapsedTime, void* pUserContext );

void InitApp();
void RenderText();


//--------------------------------------------------------------------------------------
// Точка входа в программу. Инициализирует все и переходит к обработке сообщения цикла. 
// Время простоя используется для рендеринга сцены.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
    // Включите проверку памяти во время выполнения для отладочных сборок.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
	
	// DXUT создаст и использует лучшее устройство (либо D3D9, либо D3D11)	
    // который доступен в системе в зависимости от того, какие обратные вызовы D3D установлены ниже    
	
	// Установить обратные вызовы DXUT
    DXUTSetCallbackMsgProc( MsgProc );
    DXUTSetCallbackKeyboard( OnKeyboard );
    DXUTSetCallbackFrameMove( OnFrameMove );
    DXUTSetCallbackDeviceChanging( ModifyDeviceSettings );

    DXUTSetCallbackD3D9DeviceAcceptable( IsD3D9DeviceAcceptable );
    DXUTSetCallbackD3D9DeviceCreated( OnD3D9CreateDevice );
    DXUTSetCallbackD3D9DeviceReset( OnD3D9ResetDevice );
    DXUTSetCallbackD3D9DeviceLost( OnD3D9LostDevice );
    DXUTSetCallbackD3D9DeviceDestroyed( OnD3D9DestroyDevice );
    DXUTSetCallbackD3D9FrameRender( OnD3D9FrameRender );

    DXUTSetCallbackD3D11DeviceAcceptable( IsD3D11DeviceAcceptable );
    DXUTSetCallbackD3D11DeviceCreated( OnD3D11CreateDevice );
    DXUTSetCallbackD3D11SwapChainResized( OnD3D11ResizedSwapChain );
    DXUTSetCallbackD3D11SwapChainReleasing( OnD3D11ReleasingSwapChain );
    DXUTSetCallbackD3D11DeviceDestroyed( OnD3D11DestroyDevice );
    DXUTSetCallbackD3D11FrameRender( OnD3D11FrameRender );

    InitApp();
    DXUTInit( true, true, NULL ); // разобрать командную строку, показать msgbox при ошибке, без дополнительных параметров командной строки
    DXUTSetCursorSettings( true, true );
    DXUTCreateWindow( L"SimpleSample11" );

	// Требуется только оборудование 10-го уровня, измените на D3D_FEATURE_LEVEL_11_0, 
	// чтобы требовать оборудование 11-го уровня переключиться на D3D_FEATURE_LEVEL_9_x для оборудования 10level9
    DXUTCreateDevice( D3D_FEATURE_LEVEL_10_0, true, 640, 480 );

    DXUTMainLoop(); // Войдите в цикл рендеринга DXUT

    return DXUTGetExitCode();
}


//--------------------------------------------------------------------------------------
// Инициализировать приложение
//--------------------------------------------------------------------------------------
void InitApp()
{
    g_SettingsDlg.Init( &g_DialogResourceManager );
    g_HUD.Init( &g_DialogResourceManager );
    g_SampleUI.Init( &g_DialogResourceManager );

    g_HUD.SetCallback( OnGUIEvent );
    int iY = 30;
    int iYo = 26;
    g_HUD.AddButton( IDC_TOGGLEFULLSCREEN, L"Toggle full screen", 0, iY, 170, 22 );
    g_HUD.AddButton( IDC_TOGGLEREF, L"Toggle REF (F3)", 0, iY += iYo, 170, 22, VK_F3 );
    g_HUD.AddButton( IDC_CHANGEDEVICE, L"Change device (F2)", 0, iY += iYo, 170, 22, VK_F2 );

    g_SampleUI.SetCallback( OnGUIEvent ); iY = 10;
}


//--------------------------------------------------------------------------------------
// Визуализация справки и текста статистики. 
// Эта функция использует интерфейс ID3DXFont для эффективного рендеринга текста.
//--------------------------------------------------------------------------------------
void RenderText()
{
    g_pTxtHelper->Begin();
    g_pTxtHelper->SetInsertionPos( 5, 5 );
    g_pTxtHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
    g_pTxtHelper->DrawTextLine( DXUTGetFrameStats( DXUTIsVsyncEnabled() ) );
    g_pTxtHelper->DrawTextLine( DXUTGetDeviceStats() );
    g_pTxtHelper->End();
}


//--------------------------------------------------------------------------------------
// Отклоняем любые неприемлемые устройства D3D11, возвращая false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D11DeviceAcceptable( const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo,
                                       DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
    return true;
}


//--------------------------------------------------------------------------------------
// Создаем любые ресурсы D3D11, которые не зависят от заднего буфера
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11CreateDevice( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
                                     void* pUserContext )
{
    HRESULT hr;

    ID3D11DeviceContext* pd3dImmediateContext = DXUTGetD3D11DeviceContext();
    V_RETURN( g_DialogResourceManager.OnD3D11CreateDevice( pd3dDevice, pd3dImmediateContext ) );
    V_RETURN( g_SettingsDlg.OnD3D11CreateDevice( pd3dDevice ) );
    g_pTxtHelper = new CDXUTTextHelper( pd3dDevice, pd3dImmediateContext, &g_DialogResourceManager, 15 );

    // Чтение HLSL-файла
    WCHAR str[MAX_PATH];
    V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, L"SimpleSample.hlsl" ) );

    // Скомпилировать шейдеры
    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	// Установите флаг D3DCOMPILE_DEBUG, чтобы встроить отладочную информацию в шейдеры.
    // Установка этого флага улучшает процесс отладки шейдеров, но по-прежнему позволяет
    // шейдеры должны быть оптимизированы и работать точно так, как они будут работать в
    // конфигурация выпуска этой программы.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	// Вы должны использовать самый низкий возможный профиль шейдера для вашего шейдера, 
	// чтобы включить различные уровни функций. 
	// Эти шейдеры достаточно просты, чтобы хорошо работать в самом низком возможном профиле, 
	// и будут работать на всех уровнях функций.
    ID3DBlob* pVertexShaderBuffer = NULL;
    V_RETURN( D3DX11CompileFromFile( str, NULL, NULL, "RenderSceneVS", "vs_4_0_level_9_1", dwShaderFlags, 0, NULL,
                                     &pVertexShaderBuffer, NULL, NULL ) );

    ID3DBlob* pPixelShaderBuffer = NULL;
    V_RETURN( D3DX11CompileFromFile( str, NULL, NULL, "RenderScenePS", "ps_4_0_level_9_1", dwShaderFlags, 0, NULL,
                                     &pPixelShaderBuffer, NULL, NULL ) );

    // Создание шейдеров
    V_RETURN( pd3dDevice->CreateVertexShader( pVertexShaderBuffer->GetBufferPointer(),
                                              pVertexShaderBuffer->GetBufferSize(), NULL, &g_pVertexShader11 ) );
    DXUT_SetDebugName( g_pVertexShader11, "RenderSceneVS" );

    V_RETURN( pd3dDevice->CreatePixelShader( pPixelShaderBuffer->GetBufferPointer(),
                                             pPixelShaderBuffer->GetBufferSize(), NULL, &g_pPixelShader11 ) );
    DXUT_SetDebugName( g_pPixelShader11, "RenderScenePS" );

    // Создайте макет для данных объекта
    const D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    V_RETURN( pd3dDevice->CreateInputLayout( layout, ARRAYSIZE( layout ), pVertexShaderBuffer->GetBufferPointer(),
                                             pVertexShaderBuffer->GetBufferSize(), &g_pLayout11 ) );
    DXUT_SetDebugName( g_pLayout11, "Primary" );

    // Больше не нужны шейдерные капли (blobs)
    SAFE_RELEASE( pVertexShaderBuffer );
    SAFE_RELEASE( pPixelShaderBuffer );

    // Создание объектов состояния
    D3D11_SAMPLER_DESC samDesc;
    ZeroMemory( &samDesc, sizeof(samDesc) );
    samDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samDesc.AddressU = samDesc.AddressV = samDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samDesc.MaxAnisotropy = 1;
    samDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samDesc.MaxLOD = D3D11_FLOAT32_MAX;
    V_RETURN( pd3dDevice->CreateSamplerState( &samDesc, &g_pSamLinear ) );
    DXUT_SetDebugName( g_pSamLinear, "Linear" );

    // Создание постоянных буферов
    D3D11_BUFFER_DESC cbDesc;
    ZeroMemory( &cbDesc, sizeof(cbDesc) );
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    cbDesc.ByteWidth = sizeof( CB_VS_PER_OBJECT );
    V_RETURN( pd3dDevice->CreateBuffer( &cbDesc, NULL, &g_pcbVSPerObject11 ) );
    DXUT_SetDebugName( g_pcbVSPerObject11, "CB_VS_PER_OBJECT" );

    cbDesc.ByteWidth = sizeof( CB_VS_PER_FRAME );
    V_RETURN( pd3dDevice->CreateBuffer( &cbDesc, NULL, &g_pcbVSPerFrame11 ) );
    DXUT_SetDebugName( g_pcbVSPerFrame11, "CB_VS_PER_FRAME" );

    // Создайте другие ресурсы рендеринга здесь

    // Настройте параметры обзора камеры
    D3DXVECTOR3 vecEye( 0.0f, 0.0f, -5.0f );
    D3DXVECTOR3 vecAt ( 0.0f, 0.0f, -0.0f );
    g_Camera.SetViewParams( &vecEye, &vecAt );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Создайте любые ресурсы D3D11, зависящие от заднего буфера.
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                                         const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    HRESULT hr;

    V_RETURN( g_DialogResourceManager.OnD3D11ResizedSwapChain( pd3dDevice, pBackBufferSurfaceDesc ) );
    V_RETURN( g_SettingsDlg.OnD3D11ResizedSwapChain( pd3dDevice, pBackBufferSurfaceDesc ) );

    // SНастройте параметры проекции камеры
    float fAspectRatio = pBackBufferSurfaceDesc->Width / ( FLOAT )pBackBufferSurfaceDesc->Height;
    g_Camera.SetProjParams( D3DX_PI / 4, fAspectRatio, 0.1f, 1000.0f );
    g_Camera.SetWindow( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height );
    g_Camera.SetButtonMasks( MOUSE_LEFT_BUTTON, MOUSE_WHEEL, MOUSE_MIDDLE_BUTTON );

    g_HUD.SetLocation( pBackBufferSurfaceDesc->Width - 170, 0 );
    g_HUD.SetSize( 170, 170 );
    g_SampleUI.SetLocation( pBackBufferSurfaceDesc->Width - 170, pBackBufferSurfaceDesc->Height - 300 );
    g_SampleUI.SetSize( 170, 300 );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Рендеринг сцены с помощью устройства D3D11
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime,
                                 float fElapsedTime, void* pUserContext )
{
    // Если отображается диалоговое окно настроек, визуализируйте его вместо визуализации сцены приложения.
    if( g_SettingsDlg.IsActive() )
    {
        g_SettingsDlg.OnRender( fElapsedTime );
        return;
    }       

    float ClearColor[4] = { 0.176f, 0.196f, 0.667f, 0.0f };
    ID3D11RenderTargetView* pRTV = DXUTGetD3D11RenderTargetView();
    pd3dImmediateContext->ClearRenderTargetView( pRTV, ClearColor );

    // Очистить трафарет глубины
    ID3D11DepthStencilView* pDSV = DXUTGetD3D11DepthStencilView();
    pd3dImmediateContext->ClearDepthStencilView( pDSV, D3D11_CLEAR_DEPTH, 1.0, 0 );

    // Получите матрицу проекции и вида из класса камеры
    D3DXMATRIX mWorld = *g_Camera.GetWorldMatrix();
    D3DXMATRIX mView = *g_Camera.GetViewMatrix();
    D3DXMATRIX mProj = *g_Camera.GetProjMatrix();
    D3DXMATRIX mWorldViewProjection = mWorld * mView * mProj;

    // Установите постоянные буферы
    HRESULT hr;
    D3D11_MAPPED_SUBRESOURCE MappedResource;
    V( pd3dImmediateContext->Map( g_pcbVSPerFrame11, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource ) );
    CB_VS_PER_FRAME* pVSPerFrame = ( CB_VS_PER_FRAME* )MappedResource.pData;
    pVSPerFrame->m_vLightDir = D3DXVECTOR3( 0,0.707f,-0.707f );
    pVSPerFrame->m_fTime = (float)fTime;
    pVSPerFrame->m_LightDiffuse = D3DXVECTOR4( 1.f, 1.f, 1.f, 1.f );
    pd3dImmediateContext->Unmap( g_pcbVSPerFrame11, 0 );
    pd3dImmediateContext->VSSetConstantBuffers( 1, 1, &g_pcbVSPerFrame11 );

    V( pd3dImmediateContext->Map( g_pcbVSPerObject11, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource ) );
    CB_VS_PER_OBJECT* pVSPerObject = ( CB_VS_PER_OBJECT* )MappedResource.pData;
    D3DXMatrixTranspose( &pVSPerObject->m_mWorldViewProjection, &mWorldViewProjection );
    D3DXMatrixTranspose( &pVSPerObject->m_mWorld, &mWorld );
    pVSPerObject->m_MaterialAmbientColor = D3DXVECTOR4( 0.3f, 0.3f, 0.3f, 1.0f );
    pVSPerObject->m_MaterialDiffuseColor = D3DXVECTOR4( 0.7f, 0.7f, 0.7f, 1.0f );
    pd3dImmediateContext->Unmap( g_pcbVSPerObject11, 0 );
    pd3dImmediateContext->VSSetConstantBuffers( 0, 1, &g_pcbVSPerObject11 );

    // Установить ресурсы рендеринга
    pd3dImmediateContext->IASetInputLayout( g_pLayout11 );
    pd3dImmediateContext->VSSetShader( g_pVertexShader11, NULL, 0 );
    pd3dImmediateContext->PSSetShader( g_pPixelShader11, NULL, 0 );
    pd3dImmediateContext->PSSetSamplers( 0, 1, &g_pSamLinear );

    // Рендеринг объектов здесь...

    DXUT_BeginPerfEvent( DXUT_PERFEVENTCOLOR, L"HUD / Stats" );
    g_HUD.OnRender( fElapsedTime );
    g_SampleUI.OnRender( fElapsedTime );
    RenderText();
    DXUT_EndPerfEvent();

    static DWORD dwTimefirst = GetTickCount();
    if ( GetTickCount() - dwTimefirst > 5000 )
    {    
        OutputDebugString( DXUTGetFrameStats( DXUTIsVsyncEnabled() ) );
        OutputDebugString( L"\n" );
        dwTimefirst = GetTickCount();
    }
}


//--------------------------------------------------------------------------------------
// Освобождение ресурсов D3D11, созданных в OnD3D11ResizedSwapChain.
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11ReleasingSwapChain( void* pUserContext )
{
    g_DialogResourceManager.OnD3D11ReleasingSwapChain();
}


//--------------------------------------------------------------------------------------
// Освобождение ресурсов D3D11, созданных в OnD3D11CreateDevice.
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11DestroyDevice( void* pUserContext )
{
    g_DialogResourceManager.OnD3D11DestroyDevice();
    g_SettingsDlg.OnD3D11DestroyDevice();
    DXUTGetGlobalResourceCache().OnDestroyDevice();
    SAFE_DELETE( g_pTxtHelper );

    SAFE_RELEASE( g_pVertexShader11 );
    SAFE_RELEASE( g_pPixelShader11 );
    SAFE_RELEASE( g_pLayout11 );
    SAFE_RELEASE( g_pSamLinear );

    // Удалите дополнительные ресурсы рендеринга здесь...

    SAFE_RELEASE( g_pcbVSPerObject11 );
    SAFE_RELEASE( g_pcbVSPerFrame11 );
}


//--------------------------------------------------------------------------------------
// Вызывается непосредственно перед созданием устройства D3D9 или D3D11, 
// что позволяет приложению изменять параметры устройства по мере необходимости.
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
    if( pDeviceSettings->ver == DXUT_D3D9_DEVICE )
    {
        IDirect3D9* pD3D = DXUTGetD3D9Object();
        D3DCAPS9 Caps;
        pD3D->GetDeviceCaps( pDeviceSettings->d3d9.AdapterOrdinal, pDeviceSettings->d3d9.DeviceType, &Caps );

		// Если устройство не поддерживает HW T&L или не поддерживает 
		// вершинные шейдеры 1.1 в HW затем переключиться на SWVP.
        if( ( Caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT ) == 0 ||
            Caps.VertexShaderVersion < D3DVS_VERSION( 1, 1 ) )
        {
            pDeviceSettings->d3d9.BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
        }

		// Для отладки вершинных шейдеров требуется либо REF, 
		// либо программная обработка вершин 
		// и для отладки пиксельных шейдеров требуется REF.		
#ifdef DEBUG_VS
        if( pDeviceSettings->d3d9.DeviceType != D3DDEVTYPE_REF )
        {
            pDeviceSettings->d3d9.BehaviorFlags &= ~D3DCREATE_HARDWARE_VERTEXPROCESSING;
            pDeviceSettings->d3d9.BehaviorFlags &= ~D3DCREATE_PUREDEVICE;
            pDeviceSettings->d3d9.BehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
        }
#endif
#ifdef DEBUG_PS
        pDeviceSettings->d3d9.DeviceType = D3DDEVTYPE_REF;
#endif
    }

    // Для первого созданного устройства, если оно является REF-устройством, 
	// можно дополнительно отобразить диалоговое окно с предупреждением.
    static bool s_bFirstTime = true;
    if( s_bFirstTime )
    {
        s_bFirstTime = false;
        if( ( DXUT_D3D9_DEVICE == pDeviceSettings->ver && pDeviceSettings->d3d9.DeviceType == D3DDEVTYPE_REF ) ||
            ( DXUT_D3D11_DEVICE == pDeviceSettings->ver &&
            pDeviceSettings->d3d11.DriverType == D3D_DRIVER_TYPE_REFERENCE ) )
        {
            DXUTDisplaySwitchingToREFWarning( pDeviceSettings->ver );
        }

    }

    return true;
}


//---------------------------------------------------------------------------------------------
// Обработка обновлений сцены. Это вызывается независимо от того, какой D3D API используется.
//---------------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
    // Обновите положение камеры на основе ввода пользователя
    g_Camera.FrameMove( fElapsedTime );
}


//--------------------------------------------------------------------------------------
// Обновите положение камеры на основе ввода пользователя
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
                          void* pUserContext )
{
    // Передавайте сообщения в вызовы диспетчера диалоговых ресурсов, 
	// чтобы состояние графического интерфейса обновлялось правильно.
    *pbNoFurtherProcessing = g_DialogResourceManager.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

    // Передать сообщения в диалоговое окно настроек, если оно активно
    if( g_SettingsDlg.IsActive() )
    {
        g_SettingsDlg.MsgProc( hWnd, uMsg, wParam, lParam );
        return 0;
    }

    // Дайте диалогам возможность сначала обработать сообщение
    *pbNoFurtherProcessing = g_HUD.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;
    *pbNoFurtherProcessing = g_SampleUI.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

    // Передайте все оставшиеся сообщения Windows на камеру, 
	// чтобы она могла реагировать на ввод пользователя.
    g_Camera.HandleMessages( hWnd, uMsg, wParam, lParam );

    return 0;
}


//--------------------------------------------------------------------------------------
// Обрабатывать нажатия клавиш
//--------------------------------------------------------------------------------------
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
}


//--------------------------------------------------------------------------------------
// Обрабатывает события графического интерфейса.
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
    switch( nControlID )
    {
        case IDC_TOGGLEFULLSCREEN:
            DXUTToggleFullScreen();
            break;
        case IDC_TOGGLEREF:
            DXUTToggleREF();
            break;
        case IDC_CHANGEDEVICE:
            g_SettingsDlg.SetActive( !g_SettingsDlg.IsActive() );
            break;
    }
}
