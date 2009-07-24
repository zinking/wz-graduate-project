//=================================================================================================
// File: ParallaxOcclusionMapping.cpp
//
// Author: Natalya Tatarchuk
//         ATI Research, Inc.
//         3D Application Research Group  
//
// Implementation of the algorithm as described in "Dynamic Parallax Occlusion Mapping with 
// Approximate Soft Shadows" paper, by N. Tatarchuk, ATI Research, to appear in the proceedings 
// of ACM Symposium on Interactive 3D Graphics and Games, 2006.
//                 
// Copyright (c) ATI Research, Inc. All rights reserved.
//=================================================================================================

#include "dxstdafx.h"
#include "resource.h"


//#define DEBUG_VS   // Uncomment this line to debug vertex shaders 
//#define DEBUG_PS   // Uncomment this line to debug pixel shaders 


//--------------------------------------------------------------------------------------
// Global variables and parameters
//--------------------------------------------------------------------------------------
const int s_iMAX_STRING_SIZE = 100;
const int s_iDECL_SIZE       = 56;
const int s_iNUM_TEXTURES    = 7;  // Number of loaded texturs in the program

float fAspectRatio;
float fNear = 0.1;
float fFar	= 5000;








//--------------------------------------------------------------------------------------
IDirect3DDevice9*		g_pD3DDevice	= NULL;			// A pointer to the current D3D device used for rendering
ID3DXFont*				g_pFont			= NULL;				// Font for drawing text
ID3DXSprite*			g_pSprite		= NULL;				// Sprite for batching draw text calls
bool					g_bShowHelp		= false;				// If true, it renders the UI control text
CModelViewerCamera      g_Camera;								// A model viewing camera
ID3DXEffect*			g_pEffect		= NULL;				// D3DX effect interface
ID3DXMesh*				g_pMesh = NULL;					// Mesh object
IDirect3DTexture9**		g_pBaseTextures = NULL;		// Array of base map texture surfaces
IDirect3DTexture9**		g_pNMHTextures  = NULL;		// Array of normal / height map texture surfaces
																					//       a height map in the alpha channel
IDirect3DTexture9*		g_depthTexture	= NULL;
IDirect3DTexture9*		g_noiseTexture  = NULL;
IDirect3DSurface9*		pBackBuffer		=	0;
IDirect3DSurface9*		pDepthBuffer	=	0;

CDXUTDialogResourceManager		g_DialogResourceManager;	// manager for shared resources of dialogs
CD3DSettingsDlg					g_SettingsDlg;						// Device settings dialog
CDXUTDialog						g_HUD;									// manages the 3D UI
CDXUTDialog						g_SampleUI;							// dialog for sample specific controls
D3DXMATRIXA16					g_mWorldFix;

CDXUTDirectionWidget			g_LightControl;						// Scene light
float							g_fLightScale;							// Light brightness scale parameter


//--------------------------------------------------------------------------------------
// UI scalar parameters
//--------------------------------------------------------------------------------------
const float			s_fLightScaleUIScale   = 10.0f;
const int			s_iLightScaleSliderMin = 0;
const int			s_iLightScaleSliderMax = 20;

int					g_iHeightScaleSliderMin = 0;
int					g_iHeightScaleSliderMax = 100;
float				g_fHeightScaleUIScale   = 100.0f;

const int			s_iSliderMin = 5;
const int			s_iSliderMax = 16;

//--------------------------------------------------------------------------------------
// Material properties parameters:
//--------------------------------------------------------------------------------------
D3DXCOLOR g_colorMtrlDiffuse( 1.0f, 1.0f, 1.0f, 1.0f );
D3DXCOLOR g_colorMtrlAmbient( 0.35f, 0.35f, 0.35f, 0 );
D3DXCOLOR g_colorMtrlSpecular( 1.0f, 1.0f, 1.0f, 1.0f );

float		g_fSpecularExponent(60.0f);				// Material's specular exponent
float		g_fBaseTextureRepeat(1.0f);				// The tiling factor for base and normal map textures

bool		g_bVisualizeLOD(false);					// Toggles visualization of level of detail colors
bool		g_bVisualizeMipLevel(false);			// Toggles visualization of mip level
bool		g_bDisplayShadows(true);				// Toggles display of self-occlusion based shadows
bool		g_bDisplayESO(true);					// Toggles rendering with specular or without
bool		g_bRenderPOM(true);						// Toggles whether using normal mapping or parallax occlusion mapping

int			g_nLODThreshold(3);						// The mip level id for transitioning between the full computation
													// for parallax occlusion mapping and the bump mapping computation
int			g_nSamples(16);							// The minimum number of samples for sampling the height field profile
//int			g_nMaxSamples(50);					// The maximum number of samples for sampling the height field profile
float		g_fShadowSoftening(0.58f);				// Blurring factor for the soft shadows computation
float		g_fHeightScale;							// This parameter controls the height map range for displacement mapping

//--------------------------------------------------------------------------------------
// Mesh file names for use
//--------------------------------------------------------------------------------------
LPCTSTR g_atszMeshFileName[] =
{
	L"tiny.x",
	L"teapot.x",
	L"car.x"
};

#define NUM_MESHES ( sizeof(g_atszMeshFileName) / sizeof(g_atszMeshFileName[0]) )
WCHAR*	g_strMeshFileName0 = TEXT( "tiny.x" );

WCHAR* g_strNoisefile = L"random.png";



//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
#define IDC_TOGGLEFULLSCREEN           1
#define IDC_TOGGLEREF                  3
#define IDC_CHANGEDEVICE               4
//#define IDC_HEIGHT_SCALE_SLIDER        5
//#define IDC_HEIGHT_SCALE_STATIC        6
//#define IDC_TOGGLE_SHADOWS             7
//#define IDC_SELECT_TEXTURES_COMBOBOX   8
//#define IDC_TOGGLE_SPECULAR            9
//#define IDC_SPECULAR_EXPONENT_SLIDER   10
//#define IDC_SPECULAR_EXPONENT_STATIC   11
#define IDC_NUM_SAMPLES_SLIDER     12
#define IDC_NUM_SAMPLES_STATIC     13
//#define IDC_MAX_NUM_SAMPLES_SLIDER     14
//#define IDC_MAX_NUM_SAMPLES_STATIC     15
//#define IDC_TECHNIQUE_COMBO_BOX        16
#define IDC_RELOAD_BUTTON              20
#define IDC_CHANGEMESH_COMBOBOX		21
#define MESH_OBJ 22



//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
bool			CALLBACK IsDeviceAcceptable    ( D3DCAPS9*           pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext );
bool			CALLBACK ModifyDeviceSettings  ( DXUTDeviceSettings* pDeviceSettings, const D3DCAPS9* pCaps, void* pUserContext );
HRESULT			CALLBACK OnCreateDevice        ( IDirect3DDevice9*   pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
HRESULT			CALLBACK OnResetDevice         ( IDirect3DDevice9*   pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
void			CALLBACK OnFrameMove           ( IDirect3DDevice9*   pd3dDevice, double fTime, float fElapsedTime, void* pUserContext );
void			CALLBACK OnFrameRender         ( IDirect3DDevice9*   pd3dDevice, double fTime, float fElapsedTime, void* pUserContext );
LRESULT			CALLBACK MsgProc               ( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext );
void			CALLBACK KeyboardProc          ( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext );
void			CALLBACK OnGUIEvent            ( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext );
void			CALLBACK OnLostDevice          ( void* pUserContext );
void			CALLBACK OnDestroyDevice       ( void* pUserContext );

void			InitApp();
HRESULT			LoadMesh( IDirect3DDevice9* pd3dDevice, WCHAR* strFileName, ID3DXMesh** ppMesh );
void			RenderText( double fTime );


//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a 
// message processing loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
INT WINAPI WinMain( HINSTANCE, HINSTANCE, LPSTR, int )
{
   // Enable run-time memory check for debug builds.
   #if defined(DEBUG) | defined(_DEBUG)
      _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
   #endif

   // Set the callback functions
   DXUTSetCallbackDeviceCreated  ( OnCreateDevice );
   DXUTSetCallbackDeviceReset    ( OnResetDevice );
   DXUTSetCallbackDeviceLost     ( OnLostDevice );
   DXUTSetCallbackDeviceDestroyed( OnDestroyDevice );
   DXUTSetCallbackMsgProc        ( MsgProc );
   DXUTSetCallbackKeyboard       ( KeyboardProc );
   DXUTSetCallbackFrameRender    ( OnFrameRender );
   DXUTSetCallbackFrameMove      ( OnFrameMove );

   DXUTSetCursorSettings( true, true );
   InitApp();
   DXUTInit( true, true, true ); // Parse the command line, handle the default hotkeys, and show msgboxes
   DXUTCreateWindow( L"Screen Space Ambient Occlusion" );
   DXUTCreateDevice( D3DADAPTER_DEFAULT, true, 800, 600, IsDeviceAcceptable, ModifyDeviceSettings );
   DXUTMainLoop();

   return DXUTGetExitCode();

}  

//--------------------------------------------------------------------------------------
// Select a pair of base and normal map / height map textures to use and
// setup related height map range parameters, given a texture index
// Note: all texture surfaces in g_pBaseTextures and g_pNMHTextures MUST
// be created prior to calling this function.
//--------------------------------------------------------------------------------------
//void SetPOMTextures( int iTextureIndex )
//{
//   g_nCurrentTextureID = iTextureIndex;
//
//
//
//   // Bind the new active textures to the correct texture slots in the shaders:
//   if ( g_pD3DDevice && g_pEffect )
//   {
//      HRESULT hr;
//
//      V( g_pEffect->SetTexture( "g_baseTexture", g_pBaseTextures[g_nCurrentTextureID]) );
//      V( g_pEffect->SetTexture( "g_nmhTexture",  g_pNMHTextures[g_nCurrentTextureID]) );
//
//   }  
//
//   //// Setup height map range slider parameters (need to be setup per-texture, as very height-map specific:
//   //switch( iTextureIndex )
//   //{
//   //    case POM_STONES:
//   //    {  
//   //        // Stones texture pair:
//   //        g_iHeightScaleSliderMin = 0;
//   //        g_iHeightScaleSliderMax = 10;
//   //        g_fHeightScaleUIScale   = 100.0f;
//   //        g_fHeightScale          = 0.02f;
//
//   //        g_fSpecularExponent  = 60.0f;
//   //        g_fBaseTextureRepeat = 1.0f;
//
//   //        g_nMinSamples = 8;
//   //        g_nMaxSamples = 50;
//   //        break;   
//   //    }
//   //}  
//}  
//

//--------------------------------------------------------------------------------------
// Initialize the app: initialize the light controls and the GUI 
// elements for this application.
//--------------------------------------------------------------------------------------
void InitApp()
{
   // UI elements parameters:
   const int ciPadding = 24;        // vertical padding between controls
   const int ciLeft    = 5;         // Left align border (x-coordinate) of the controls
   const int ciWidth   = 125;       // widget width
   const int ciHeight  = 22;        // widget height

   // Initialize the light control:
   g_LightControl.SetLightDirection( D3DXVECTOR3( sinf( D3DX_PI * 2 - D3DX_PI / 6 ), 
                                                  0, 
                                                 -cosf( D3DX_PI * 2 - D3DX_PI / 6 )));
   g_fLightScale  = 1.0f;

   // Initialize POM textures and height map parameters:
   //SetPOMTextures( POM_STONES );

   // Initialize dialogs // 
   g_SettingsDlg.Init( &g_DialogResourceManager );
   g_HUD.Init( &g_DialogResourceManager );
   g_SampleUI.Init( &g_DialogResourceManager );

   // Initialize user interface elements //

   // Top toggle buttons //
   g_HUD.SetCallback( OnGUIEvent );
   
   int iY = 10; 
   g_HUD.AddButton( IDC_TOGGLEFULLSCREEN, L"Toggle full screen", ciLeft, iY, ciWidth, ciHeight );
   iY += ciPadding;
   g_HUD.AddButton( IDC_TOGGLEREF, L"Toggle REF (F3)", ciLeft, iY, ciWidth, ciHeight );
   iY += ciPadding;
   g_HUD.AddButton( IDC_CHANGEDEVICE, L"Change device (F2)", ciLeft, iY, ciWidth, ciHeight, VK_F2 );
   iY += ciPadding;

   // Reload button only works in debug:
   #ifdef _DEBUG
      g_HUD.AddButton( IDC_RELOAD_BUTTON, L"Reload effect", ciLeft, iY, ciWidth, ciHeight );
   #endif   
   
   iY = 10; 
   g_SampleUI.SetCallback( OnGUIEvent );

   WCHAR sz[s_iMAX_STRING_SIZE];
   
   // Slider parameters:
   const int ciSliderLeft  = 15;
   const int ciSliderWidth = 100;

   // Technique selection combo box
   CDXUTComboBox *pMeshComboBox = NULL;
   g_SampleUI.AddComboBox( IDC_CHANGEMESH_COMBOBOX, ciLeft, iY, 200, 24, L'O', false, &pMeshComboBox );
   if ( pMeshComboBox )
   {
      pMeshComboBox->SetDropHeight( 100 );
	  for ( int i=0; i < NUM_MESHES; i++ ){
			pMeshComboBox->AddItem( g_atszMeshFileName[i],(LPVOID) (MESH_OBJ+i) );
	  }
      //pTechniqueComboBox->AddItem( L"Parallax Occlusion Mapping",              (LPVOID)POM     );
      //pTechniqueComboBox->AddItem( L"Bump Mapping",                            (LPVOID)BUMPMAP );
      //pTechniqueComboBox->AddItem( L"Parallax Mapping with Offset Limiting",   (LPVOID)PM      );
      
   }  

   // Add control for height range //
   //iY += ciPadding;
   //StringCchPrintf( sz, s_iMAX_STRING_SIZE, L"Height scale: %0.2f", g_fHeightScale ); 
   //g_SampleUI.AddStatic( IDC_HEIGHT_SCALE_STATIC, sz, ciLeft, iY, ciWidth, ciHeight );
   
   /*iY += ciPadding;
   g_SampleUI.AddSlider( IDC_HEIGHT_SCALE_SLIDER, ciSliderLeft, iY, ciSliderWidth, ciHeight, g_iHeightScaleSliderMin, g_iHeightScaleSliderMax, 
                         (int)( g_fHeightScale * g_fHeightScaleUIScale ) );*/
   
   // Texture selection combo box:
   //iY += ciPadding;
   //CDXUTComboBox *pTextureComboBox = NULL;
   //g_SampleUI.AddComboBox( IDC_SELECT_TEXTURES_COMBOBOX, ciLeft, iY, 200, 24, L'O', false, &pTextureComboBox );
   //if( pTextureComboBox )
   //{
   //   pTextureComboBox->SetDropHeight( 100 );
   //   //pTextureComboBox->AddItem( L"Stones",                           (LPVOID)POM_STONES      );
   //   //pTextureComboBox->AddItem( L"Rocks",                            (LPVOID)POM_ROCKS       );
   //   //pTextureComboBox->AddItem( L"Wall",                             (LPVOID)POM_WALL        );
   //   //pTextureComboBox->AddItem( L"Sphere, Triangle, Torus, Pyramid", (LPVOID)POM_FOUR_BUMPS  );
   //   //pTextureComboBox->AddItem( L"Bumps",                            (LPVOID)POM_BUMPS       );
   //   //pTextureComboBox->AddItem( L"Dents",                            (LPVOID)POM_DENTS       );
   //   //pTextureComboBox->AddItem( L"Relief",                           (LPVOID)POM_SAINT       );

   //}  
   
   // Toggle shadows checkbox
   /*iY += ciPadding;
   g_SampleUI.AddCheckBox( IDC_TOGGLE_SHADOWS, L"Toggle self-occlusion shadows rendering: ON",
                            ciLeft, iY, 350, 24, g_bDisplayShadows, L'C', false );*/

   // Toggle specular checkbox
   /*iY += ciPadding;
   g_SampleUI.AddCheckBox( IDC_TOGGLE_SPECULAR, L"Toggle ESO: ON",
                            ciLeft, iY, 350, 24, g_bDisplayESO(), L'C', false );*/

   
   // Specular exponent slider:
   /*iY += ciPadding;
   StringCchPrintf( sz, s_iMAX_STRING_SIZE, L"Specular exponent: %0.0f", g_fSpecularExponent ); 
   g_SampleUI.AddStatic( IDC_SPECULAR_EXPONENT_STATIC, sz, ciLeft, iY, ciWidth, ciHeight );*/
   
   /*iY += ciPadding;
   g_SampleUI.AddSlider( IDC_SPECULAR_EXPONENT_SLIDER, ciSliderLeft, iY, ciSliderWidth, ciHeight, 
                         s_iSliderMin, s_iSliderMax, (int) g_fSpecularExponent );*/

   // Number of samples: minimum
   iY += ciPadding;
   StringCchPrintf( sz, s_iMAX_STRING_SIZE, L"Number of samples: %d", g_nSamples ); 
   g_SampleUI.AddStatic( IDC_NUM_SAMPLES_STATIC, sz, ciLeft, iY, ciWidth * 2, ciHeight );
   g_SampleUI.GetStatic( IDC_NUM_SAMPLES_STATIC )->GetElement( 0 )->dwTextFormat = DT_LEFT | DT_TOP | DT_WORDBREAK;
   
   iY += ciPadding;
   g_SampleUI.AddSlider( IDC_NUM_SAMPLES_SLIDER, ciSliderLeft, iY, ciSliderWidth, ciHeight, 
                         s_iSliderMin, s_iSliderMax, g_nSamples );

   // Number of samples: maximum
  /* iY += ciPadding;
   StringCchPrintf( sz, s_iMAX_STRING_SIZE, L"Maximum number of samples: %d", g_nMaxSamples ); 
   g_SampleUI.AddStatic( IDC_MAX_NUM_SAMPLES_STATIC, sz, ciLeft, iY, ciWidth * 2, ciHeight );
   g_SampleUI.GetStatic( IDC_MAX_NUM_SAMPLES_STATIC )->GetElement( 0 )->dwTextFormat = DT_LEFT | DT_TOP | DT_WORDBREAK;
   */
  /* 
   iY += ciPadding;
   g_SampleUI.AddSlider( IDC_MAX_NUM_SAMPLES_SLIDER, ciSliderLeft, iY, ciSliderWidth, ciHeight, 
                         s_iSliderMin, s_iSliderMax, g_nMaxSamples );*/
   
}  


//--------------------------------------------------------------------------------------
// Called during device initialization, this code checks the device for some 
// minimum set of capabilities, and rejects those that don't pass by 
// returning E_FAIL.
//--------------------------------------------------------------------------------------
bool CALLBACK IsDeviceAcceptable( D3DCAPS9*  pCaps, 
                                  D3DFORMAT  AdapterFormat, 
                                  D3DFORMAT  BackBufferFormat, 
                                  bool       bWindowed, 
                                  void*      pUserContext )
{
   // The ParallaxOcculsionMapping technique requires PS 3.0 so reject any device that 
   // doesn't support at least PS 3.0.  Typically, the app would fallback but 
   // ParallaxOcculsionMapping is the purpose of this sample
   if( pCaps->PixelShaderVersion < D3DPS_VERSION( 3, 0 ) )
   {
      return false;
   }

   // Skip backbuffer formats that don't support alpha blending
   IDirect3D9* pD3D = DXUTGetD3DObject(); 
   if ( FAILED( pD3D->CheckDeviceFormat( pCaps->AdapterOrdinal, 
                                         pCaps->DeviceType,
                                         AdapterFormat, 
                                         D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING, 
                                         D3DRTYPE_TEXTURE, 
                                         BackBufferFormat ) ) )
   {
      return false;
   }

   return true;

}  


//--------------------------------------------------------------------------------------
// Called right before creating a D3D9 device, allowing the app to modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, 
                                    const D3DCAPS9*     pCaps, 
                                    void*               pUserContext )
{
   // If device doesn't support VS 3.0 then switch to SWVP
   if( pDeviceSettings->DeviceType != D3DDEVTYPE_REF && 
       ((pCaps->DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) == 0 || pCaps->VertexShaderVersion < D3DVS_VERSION(3, 0)) )
   {
      pDeviceSettings->BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
   }

   // Debugging vertex shaders requires either REF or software vertex processing 
   // and debugging pixel shaders requires REF.  
   #ifdef DEBUG_VS
      if ( pDeviceSettings->DeviceType != D3DDEVTYPE_REF )
      {
         pDeviceSettings->BehaviorFlags &= ~D3DCREATE_HARDWARE_VERTEXPROCESSING;
         pDeviceSettings->BehaviorFlags &= ~D3DCREATE_PUREDEVICE;                            
         pDeviceSettings->BehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
      }
   #endif
   #ifdef DEBUG_PS
      pDeviceSettings->DeviceType = D3DDEVTYPE_REF;
   #endif

   // Turn off VSynch
   pDeviceSettings->pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

   pDeviceSettings->pp.AutoDepthStencilFormat = D3DFMT_D24S8;
   

   // For the first device created if its a REF device, optionally display a warning dialog box
   static bool s_bFirstTime = true;   
   if( s_bFirstTime )
   {
      s_bFirstTime = false;
      if ( pDeviceSettings->DeviceType == D3DDEVTYPE_REF )
         DXUTDisplaySwitchingToREFWarning();
   }  

   return true;
}  

//--------------------------------------------------------------------------------------
// Loads the effect file from disk. Note: D3D device must be created and set 
// prior to calling this method.
//--------------------------------------------------------------------------------------
HRESULT LoadEffectFile()
{
   SAFE_RELEASE( g_pEffect );

   if ( g_pD3DDevice == NULL )
      return E_FAIL;
   
   DWORD dwShaderFlags = D3DXFX_NOT_CLONEABLE;

    #if defined( DEBUG ) || defined( _DEBUG )
    // Set the D3DXSHADER_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DXSHADER_DEBUG;
    #endif

   #ifdef DEBUG_VS
      dwShaderFlags |= D3DXSHADER_FORCE_VS_SOFTWARE_NOOPT;
   #endif
   #ifdef DEBUG_PS
      dwShaderFlags |= D3DXSHADER_FORCE_PS_SOFTWARE_NOOPT;
   #endif

   WCHAR str[MAX_PATH];
   HRESULT hr;
   V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, L"AmbientOcclusion.fx" ) );

   ID3DXBuffer *pErrors = NULL;
   hr = D3DXCreateBuffer(1024, &pErrors );
   hr = D3DXCreateEffectFromFile( g_pD3DDevice, str, NULL, NULL, dwShaderFlags, NULL, &g_pEffect, &pErrors );

   if ( FAILED( hr ) )
   {
      // Output shader compilation errors to the shell:
      CHAR *pErrorStr = ( CHAR* ) pErrors->GetBufferPointer();
      printf( "%s\n", pErrorStr );

      return E_FAIL;
   } 

   SAFE_RELEASE( pErrors );

   return S_OK;

}  

//--------------------------------------------------------------------------------------
// Create any D3D9 resources that will live through a device reset (D3DPOOL_MANAGED)
// and aren't tied to the back buffer size
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnCreateDevice( IDirect3DDevice9*       pd3dDevice,
                                 const D3DSURFACE_DESC*  pBackBufferSurfaceDesc, 
                                 void*                   pUserContext )
{
   g_pD3DDevice = pd3dDevice;

   HRESULT hr;

   V_RETURN( g_DialogResourceManager.OnCreateDevice( pd3dDevice ) );
   V_RETURN( g_SettingsDlg.OnCreateDevice( pd3dDevice ) );

   // Initialize the font 
   V_RETURN( D3DXCreateFont( pd3dDevice, 15, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET, 
                             OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, 
                             L"Arial", &g_pFont ) );

   // Load the mesh 
   V_RETURN( LoadMesh( pd3dDevice, g_strMeshFileName0, &g_pMesh ) );

   D3DXVECTOR3* pData; 
   D3DXVECTOR3 vCenter;
   FLOAT fObjectRadius;
   V( g_pMesh->LockVertexBuffer( 0, (LPVOID*) &pData ) );
   //V( D3DXComputeBoundingSphere( pData, g_pMesh->GetNumVertices(), s_iDECL_SIZE, &vCenter, &fObjectRadius ) );
   V( D3DXComputeBoundingSphere( pData, g_pMesh->GetNumVertices(), D3DXGetFVFVertexSize( g_pMesh->GetFVF() ), &vCenter, &fObjectRadius ) );
   V( g_pMesh->UnlockVertexBuffer() );

   // Align the starting frame of the mesh to be slightly toward the viewer yet to 
   // see the grazing angles:                                                      
   D3DXMatrixTranslation( &g_mWorldFix, -vCenter.x, -vCenter.y, -vCenter.z );
   D3DXMATRIXA16 mRotation;
   D3DXMatrixRotationY( &mRotation, -D3DX_PI / 3.0f );
   g_mWorldFix *= mRotation;
   D3DXMatrixRotationX( &mRotation, D3DX_PI / 3.0f );
   g_mWorldFix *= mRotation;

   // Initialize the light control 
   V_RETURN( CDXUTDirectionWidget::StaticOnCreateDevice( pd3dDevice ) );
   g_LightControl.SetRadius( fObjectRadius );
   
   V_RETURN( LoadEffectFile() );

   // Load all textures used by the program from disk 
   WCHAR str[MAX_PATH];



   V_RETURN( DXUTFindDXSDKMediaFileCch( str , MAX_PATH, g_strNoisefile ) );
   V_RETURN( D3DXCreateTextureFromFileEx( pd3dDevice, str , D3DX_DEFAULT, D3DX_DEFAULT, 
	   D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, 
	   D3DX_DEFAULT, D3DX_DEFAULT, 0, 
	   NULL, NULL, &g_noiseTexture ) );

   pd3dDevice->CreateTexture( 800 , 600 ,0 ,D3DUSAGE_RENDERTARGET,D3DFMT_A32B32G32R32F,D3DPOOL_DEFAULT,&g_depthTexture,0);
   //pd3dDevice->CreateTexture( 800 , 600, 0, D3DUSAGE_DEPTHSTENCIL, D3DFMT_D32, D3DPOOL_DEFAULT, &g_depthTexture, 0 );

   pd3dDevice->GetRenderTarget(0,&pBackBuffer);
   g_depthTexture->GetSurfaceLevel(0,&pDepthBuffer);	
    V( g_pEffect->SetTexture( "g_DepthTexture",  g_depthTexture ) );
	V( g_pEffect->SetTexture( "g_noiseTexture",   g_noiseTexture ) );


	pd3dDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);



   //set up corner frustrum parameter in effect
	float farY = (float)tan(D3DX_PI / 3.0 / 2.0) * fFar;
	float farX = farY * fAspectRatio;
	D3DXVECTOR4 vFustrumCorner( farX, farY, fFar,1);
	V( g_pEffect->SetVector( "g_vFustrumCorner",	&vFustrumCorner ) );
	V( g_pEffect->SetFloat("far",	fFar) );
	V( g_pEffect->SetFloat("near", fNear) );
  
   // Setup the camera's view parameters 
   D3DXVECTOR3 vecEye( 0.0f, 0.0f, -15.0f );
   D3DXVECTOR3 vecAt ( 0.0f, 0.0f, -0.0f );
   g_Camera.SetViewParams( &vecEye, &vecAt );
   g_Camera.SetRadius( fObjectRadius * 3.0f, fObjectRadius * 0.5f, fObjectRadius * 10.0f );

   return S_OK;
}  

//--------------------------------------------------------------------------------------
// This function loads the mesh and ensures the mesh has normals; it also 
// optimizes the mesh for the graphics card's vertex cache, which improves 
// performance by organizing the internal triangle list for less cache misses.
//--------------------------------------------------------------------------------------
HRESULT LoadMesh( IDirect3DDevice9* pd3dDevice, WCHAR* strFileName, ID3DXMesh** ppMesh )
{
	ID3DXMesh* pMesh = NULL;
	WCHAR str[MAX_PATH];
	HRESULT hr;

	// Load the mesh with D3DX and get back a ID3DXMesh*.  For this
	// sample we'll ignore the X file's embedded materials since we know 
	// exactly the model we're loading.  See the mesh samples such as
	// "OptimizedMesh" for a more generic mesh loading example.
	V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, strFileName ) );
	V_RETURN( D3DXLoadMeshFromX(str, D3DXMESH_MANAGED, pd3dDevice, NULL, NULL, NULL, NULL, &pMesh) );

	DWORD *rgdwAdjacency = NULL;

	// Make sure there are normals which are required for lighting
	if( !(pMesh->GetFVF() & D3DFVF_NORMAL) )
	{
		ID3DXMesh* pTempMesh;
		V( pMesh->CloneMeshFVF( pMesh->GetOptions(), 
			pMesh->GetFVF() | D3DFVF_NORMAL, 
			pd3dDevice, &pTempMesh ) );
		V( D3DXComputeNormals( pTempMesh, NULL ) );

		SAFE_RELEASE( pMesh );
		pMesh = pTempMesh;
	}

	// Optimize the mesh for this graphics card's vertex cache 
	// so when rendering the mesh's triangle list the vertices will 
	// cache hit more often so it won't have to re-execute the vertex shader 
	// on those vertices so it will improve perf.     
	rgdwAdjacency = new DWORD[pMesh->GetNumFaces() * 3];
	if( rgdwAdjacency == NULL )
		return E_OUTOFMEMORY;
	V( pMesh->GenerateAdjacency(1e-6f,rgdwAdjacency) );
	V( pMesh->OptimizeInplace(D3DXMESHOPT_VERTEXCACHE, rgdwAdjacency, NULL, NULL, NULL) );
	delete []rgdwAdjacency;

	*ppMesh = pMesh;

	return S_OK;

}  


//--------------------------------------------------------------------------------------
// Create any D3D9 resources that won't live through a device reset (D3DPOOL_DEFAULT) 
// or that are tied to the back buffer size 
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnResetDevice( IDirect3DDevice9*        pd3dDevice, 
                                const D3DSURFACE_DESC*   pBackBufferSurfaceDesc, 
                                void*                    pUserContext )
{
   HRESULT hr;

   V_RETURN( g_DialogResourceManager.OnResetDevice() );
   V_RETURN( g_SettingsDlg.OnResetDevice() );

   if ( g_pFont )
   {
      V_RETURN( g_pFont->OnResetDevice() );
   }
   if ( g_pEffect )
   {
      V_RETURN( g_pEffect->OnResetDevice() );
   }

   // Create a sprite to help batch calls when drawing many lines of text
   V_RETURN( D3DXCreateSprite( pd3dDevice, &g_pSprite ) );

   g_LightControl.OnResetDevice( pBackBufferSurfaceDesc  );
   
   // Setup the camera's projection parameters
   fAspectRatio = pBackBufferSurfaceDesc->Width / (FLOAT) pBackBufferSurfaceDesc->Height;

   g_Camera.SetProjParams( D3DX_PI / 3, fAspectRatio, fNear, fFar );
   g_Camera.SetWindow( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height );
   g_Camera.SetButtonMasks( MOUSE_LEFT_BUTTON, MOUSE_WHEEL, MOUSE_MIDDLE_BUTTON );

   const int ciHUDLeftBorderOffset		= 170;
   const int ciHUDVerticalSize			= 170;
   const int ciUILeftBorderOffset		= 0;
   const int ciUITopBorderOffset		= 40;
   const int ciUIVerticalSize			= 600;
   const int ciUIHorizontalSize			= 300;

   g_HUD.SetLocation( pBackBufferSurfaceDesc->Width - ciHUDLeftBorderOffset, 0 );
   g_HUD.SetSize( ciHUDLeftBorderOffset, ciHUDVerticalSize );

   g_SampleUI.SetLocation( ciUILeftBorderOffset, ciUITopBorderOffset );
   g_SampleUI.SetSize( ciUIHorizontalSize, ciUIVerticalSize );

   return S_OK;

}  


//--------------------------------------------------------------------------------------
// Handle updates to the scene.  This is called regardless of which D3D API is used
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( IDirect3DDevice9* pd3dDevice, 
                           double            fTime, 
                           float             fElapsedTime, 
                           void*             pUserContext )
{
   // Update the camera's position based on user input 
   g_Camera.FrameMove( fElapsedTime );

}  


//--------------------------------------------------------------------------------------
// Render the scene using the D3D9 device
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameRender( IDirect3DDevice9* pd3dDevice, 
                             double            fTime, 
                             float             fElapsedTime, 
                             void*             pUserContext )
{
   // If the settings dialog is being shown, then
   // render it instead of rendering the app's scene
   if ( g_SettingsDlg.IsActive() )
   {
      g_SettingsDlg.OnRender( fElapsedTime );
      return;

   }  

   HRESULT hr;
   D3DXMATRIXA16 mWorldViewProjection;
	D3DXMATRIXA16 mWorldView;
   D3DXVECTOR3 vLightDir;
   D3DXCOLOR    vLightDiffuse;
   UINT iPass, cPasses;

   D3DXMATRIXA16 mWorld;
   D3DXMATRIXA16 mView;
   D3DXMATRIXA16 mProjection;
   D3DXMATRIXA16 mITWorldView;

   // Get the projection & view matrix from the camera class
   mWorld			= g_mWorldFix * (*g_Camera.GetWorldMatrix());
   mProjection		= (*g_Camera.GetProjMatrix());
   mView			= (*g_Camera.GetViewMatrix());

   mWorldViewProjection = mWorld * mView * mProjection;
   mWorldView				 = mWorld * mView;
   //mITWorldView	=  Matrix4.transpose( D3DXMatrixInverse( mWorldView ) );
   D3DXMATRIXA16 mTemp;
   D3DXMatrixInverse( &mTemp,NULL, &mWorldView );
   D3DXMatrixTranspose( &mITWorldView, &mTemp );


   // Get camera position:
   D3DXVECTOR4 vEye;
   D3DXVECTOR3 vTemp = ( *g_Camera.GetEyePt() );
   vEye.x = vTemp.x; 
   vEye.y = vTemp.y; 
   vEye.z = vTemp.z; 
   vEye.w = 1.0;

  

    // Render the scene first time to create depth texture for second pass use
   pd3dDevice->SetRenderTarget(0,pDepthBuffer);
   V( pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL , D3DXCOLOR(0.0f,0.0,0.0,0.0), 1.0f, 0 ) );
   V( pd3dDevice->BeginScene() );

   V( g_pEffect->SetMatrix( "g_mWorldViewProjection",	&mWorldViewProjection ) );
   V( g_pEffect->SetMatrix( "g_mWorld",					&mWorld ) );
   V( g_pEffect->SetMatrix( "g_mView",					&mView ));
   V( g_pEffect->SetMatrix( "g_mWorldView",				&mWorldView ));
    V( g_pEffect->SetMatrix( "g_mITWorldView",			&mITWorldView ));
   V( g_pEffect->SetVector( "g_vEye",					&vEye ) );
 
   V( g_pEffect->SetTechnique( "RenderSceneWithSSAO" ) ); 
   V( g_pEffect->Begin( &cPasses, 0 ) );
   V( g_pEffect->BeginPass( 0 ) );
   V( g_pMesh->DrawSubset(0) );
   V( g_pEffect->EndPass( ) );
   V( g_pEffect->End() );

   V( pd3dDevice->EndScene() );

   // Render the scene second time to get the AO scene
   pd3dDevice->SetRenderTarget(0,pBackBuffer);
   V( pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, D3DXCOLOR(0.25f,0.25,0.0,0.0), 1.0f, 0 ) );
   V( pd3dDevice->BeginScene() );

  // Render the light arrow so the user can visually see the light dir
  D3DXCOLOR arrowColor = D3DXCOLOR( 1,1,0,1 );
  V( g_LightControl.OnRender( arrowColor, &mView, &mProjection, g_Camera.GetEyePt() ) );
  vLightDir       = g_LightControl.GetLightDirection();
  vLightDiffuse = g_fLightScale * D3DXCOLOR( 1, 1, 1, 1 );
  V( g_pEffect->SetValue( "g_LightDir",			&vLightDir,     sizeof( D3DXVECTOR3 ) ) );


  // Update the effect's variables.  Instead of using strings, it would 
  V( g_pEffect->SetMatrix( "g_mWorldViewProjection",	&mWorldViewProjection ) );
  V( g_pEffect->SetMatrix( "g_mWorld",					&mWorld ) );
  V( g_pEffect->SetMatrix( "g_mView",					&mView ));
  V( g_pEffect->SetMatrix( "g_mProjection",				&mProjection ));
  V( g_pEffect->SetMatrix( "g_mWorldView",				&mWorldView ));
  V( g_pEffect->SetMatrix( "g_mITWorldView",			&mITWorldView ));
  V( g_pEffect->SetVector( "g_vEye",					&vEye ) );
  V( g_pEffect->SetInt(  "g_nSamples",				g_nSamples) );



  V( g_pEffect->SetTechnique( "RenderSceneWithSSAO" ) ); 
  V( g_pEffect->Begin( &cPasses, 0 ) );
  
  V( g_pEffect->BeginPass( 1 ) );
  V( g_pMesh->DrawSubset(0) );
  V( g_pEffect->EndPass( ) );
  V( g_pEffect->End() );

   g_HUD.OnRender( fElapsedTime ); 
   g_SampleUI.OnRender( fElapsedTime );

   RenderText( fTime );

   V( pd3dDevice->EndScene() );

  
}  


//--------------------------------------------------------------------------------------
// Render the help and statistics text. This function uses the ID3DXFont 
// interface for efficient text rendering.
//--------------------------------------------------------------------------------------
void RenderText( double fTime )
{
   CDXUTTextHelper txtHelper( g_pFont, g_pSprite, 15 );

   // Output statistics
   txtHelper.Begin();
   txtHelper.SetInsertionPos( 2, 0 );
   txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
   txtHelper.DrawTextLine( DXUTGetFrameStats( true ) );
   txtHelper.DrawTextLine( DXUTGetDeviceStats() );

   txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ) );
   
   // Draw help
   if ( g_bShowHelp )
   {
      const D3DSURFACE_DESC* pd3dsdBackBuffer = DXUTGetBackBufferSurfaceDesc();
      txtHelper.SetInsertionPos( 2, pd3dsdBackBuffer->Height - 15 * 6 );
      txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 0.75f, 0.0f, 1.0f ) );
      txtHelper.DrawTextLine( L"Controls:" );

      txtHelper.SetInsertionPos( 20, pd3dsdBackBuffer->Height - 15 * 5 );
      txtHelper.DrawTextLine(	L"Rotate model: Left mouse button\n"
											L"Rotate light: Right mouse button\n"
											L"Rotate camera: Middle mouse button\n"
											L"Zoom camera: Mouse wheel scroll\n" );

      txtHelper.SetInsertionPos( 250, pd3dsdBackBuffer->Height - 15 * 5 );
      txtHelper.DrawTextLine(	L"Hide help: F1\n" 
											L"Quit: ESC\n" );
   }
   else
   {
      txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ) );
      txtHelper.DrawTextLine( L"Press F1 for help" );
   }
   txtHelper.End();
}  


//--------------------------------------------------------------------------------------
// Handle messages to the application
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND   hWnd, 
                          UINT   uMsg, 
                          WPARAM wParam, 
                          LPARAM lParam, 
                          bool*  pbNoFurtherProcessing, 
                          void*  pUserContext )
{
   // Always allow dialog resource manager calls to handle global messages
   // so GUI state is updated correctly
   *pbNoFurtherProcessing = g_DialogResourceManager.MsgProc( hWnd, uMsg, wParam, lParam );
   if ( *pbNoFurtherProcessing )
   {
      return 0;
   }

   if ( g_SettingsDlg.IsActive() )
   {
      g_SettingsDlg.MsgProc( hWnd, uMsg, wParam, lParam );
      return 0;
   }

   // Give the dialogs a chance to handle the message first
   *pbNoFurtherProcessing = g_HUD.MsgProc( hWnd, uMsg, wParam, lParam );
   if ( *pbNoFurtherProcessing )
   {
      return 0;
   }
   *pbNoFurtherProcessing = g_SampleUI.MsgProc( hWnd, uMsg, wParam, lParam );
   if ( *pbNoFurtherProcessing )
   {
      return 0;
   }

   // Pass the message to be handled to the light control:
   g_LightControl.HandleMessages( hWnd, uMsg, wParam, lParam );

   // Pass all remaining windows messages to camera so it can respond to user input
   g_Camera.HandleMessages( hWnd, uMsg, wParam, lParam );

   return 0;
}  


//--------------------------------------------------------------------------------------
// Handle key presses
//--------------------------------------------------------------------------------------
void CALLBACK KeyboardProc( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
   if ( bKeyDown )
   {
      switch( nChar )
      {
         case VK_F1: 
            {
               g_bShowHelp = !g_bShowHelp; 
            }
            break;
      }
   }  
}  


//--------------------------------------------------------------------------------------
// Switches to a specified pair of the POM texture pair from user input
//--------------------------------------------------------------------------------------
//void SwitchPOMTextures( DXUTComboBoxItem* pSelectedItem)
//{
//   //SetPOMTextures( (int)(INT_PTR) pSelectedItem->pData );
//   WCHAR strLabel[s_iMAX_STRING_SIZE];
//
//   // Update the appropriate slider controls' bounds and values:
//   CDXUTSlider* pHeightRangeSlider = g_SampleUI.GetSlider( IDC_HEIGHT_SCALE_SLIDER );
//   if ( pHeightRangeSlider )
//   {
//      pHeightRangeSlider->SetRange( g_iHeightScaleSliderMin, g_iHeightScaleSliderMax );
//      pHeightRangeSlider->SetValue( (int) ( g_fHeightScale * g_fHeightScaleUIScale ));
//
//   } 
//
//   // Update the static parameter value:
//   CDXUTStatic* pHeightRangeStatic = g_SampleUI.GetStatic( IDC_HEIGHT_SCALE_STATIC );
//   if ( pHeightRangeStatic )
//   {
//      StringCchPrintf( strLabel, s_iMAX_STRING_SIZE, L"Height scale: %0.2f", g_fHeightScale ); 
//      pHeightRangeStatic->SetText( strLabel );
//
//   } 
//
//   // Update the appropriate slider controls' bounds and values:
//   CDXUTSlider* pSpecularSlider = g_SampleUI.GetSlider( IDC_SPECULAR_EXPONENT_SLIDER );
//   if ( pSpecularSlider )
//   {
//      pSpecularSlider->SetValue( (int) g_fSpecularExponent );
//
//   } 
//
//   CDXUTStatic* pSpecularStatic = g_SampleUI.GetStatic( IDC_SPECULAR_EXPONENT_STATIC );
//   if ( pSpecularStatic )
//   {
//      StringCchPrintf( strLabel, s_iMAX_STRING_SIZE, L"Specular exponent: %0.0f", g_fSpecularExponent ); 
//      pSpecularStatic->SetText( strLabel );
//
//   } 
//
//   // Mininum number of samples slider
//   CDXUTSlider* pSlider = g_SampleUI.GetSlider( IDC_NUM_SAMPLES_SLIDER );
//   if ( pSlider )
//   {
//      pSlider->SetValue( g_nSamples );
//
//   } 
//
//   CDXUTStatic* pStatic = g_SampleUI.GetStatic( IDC_NUM_SAMPLES_STATIC );
//   if ( pStatic )
//   {
//      StringCchPrintf( strLabel, s_iMAX_STRING_SIZE, L"Number of samples: %d", g_nSamples ); 
//      pStatic->SetText( strLabel );
//
//   } 
//
//   // Maximum number of samples slider
//   /*CDXUTSlider* pMaxSlider = g_SampleUI.GetSlider( IDC_MAX_NUM_SAMPLES_SLIDER );
//   if ( pMaxSlider )
//   {
//      pMaxSlider->SetValue( g_nMaxSamples );
//
//   } 
//
//   CDXUTStatic* pMaxStatic = g_SampleUI.GetStatic( IDC_MAX_NUM_SAMPLES_STATIC );
//   if ( pMaxStatic )
//   {
//      StringCchPrintf( strLabel, s_iMAX_STRING_SIZE, L"Maximum number of samples: %d", g_nMaxSamples ); 
//      pMaxStatic->SetText( strLabel );
//   } */
//} 
//

//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
    switch( nControlID )
    {
        case IDC_TOGGLEFULLSCREEN: 
        {
            DXUTToggleFullScreen(); 
            break;  
        }

        case IDC_TOGGLEREF:        
        {
            DXUTToggleREF(); 
            break;
        }

		case IDC_CHANGEMESH_COMBOBOX:
			/*if( ++g_nCurrMesh == NUM_MESHES )
				g_nCurrMesh = 0;*/
			break;

		case IDC_NUM_SAMPLES_SLIDER:
		{
			g_nSamples = g_SampleUI.GetSlider( IDC_NUM_SAMPLES_SLIDER )->GetValue();
			WCHAR strLabel[s_iMAX_STRING_SIZE];
			StringCchPrintf( strLabel, s_iMAX_STRING_SIZE, L"Number of samples: %d", g_nSamples ); 
			g_SampleUI.GetStatic( IDC_NUM_SAMPLES_STATIC )->SetText( strLabel );
			break;
		}

        //case IDC_CHANGEDEVICE:     
        //{
        //    g_SettingsDlg.SetActive( !g_SettingsDlg.IsActive() ); 
        //    break; 
        //}

        //case IDC_SPECULAR_EXPONENT_SLIDER:
        //{
        //    g_fSpecularExponent = (float) ( g_SampleUI.GetSlider( IDC_SPECULAR_EXPONENT_SLIDER )->GetValue() );
        //    WCHAR sz[s_iMAX_STRING_SIZE];
        //    StringCchPrintf( sz, s_iMAX_STRING_SIZE, L"Specular exponent: %0.0f", g_fSpecularExponent ); 
        //    g_SampleUI.GetStatic( IDC_SPECULAR_EXPONENT_STATIC )->SetText( sz );
        //    break;
        //}

        //case IDC_HEIGHT_SCALE_SLIDER:
        //{
        //    g_fHeightScale = (float) ( g_SampleUI.GetSlider( IDC_HEIGHT_SCALE_SLIDER )->GetValue() / g_fHeightScaleUIScale );
        //    WCHAR sz[s_iMAX_STRING_SIZE];
        //    StringCchPrintf( sz, s_iMAX_STRING_SIZE, L"Height scale: %0.2f", g_fHeightScale ); 
        //    g_SampleUI.GetStatic( IDC_HEIGHT_SCALE_STATIC )->SetText( sz );
        //    break; 
        //}



        //case IDC_MAX_NUM_SAMPLES_SLIDER:
        //{
        //    g_nMaxSamples = g_SampleUI.GetSlider( IDC_MAX_NUM_SAMPLES_SLIDER )->GetValue();

        //    WCHAR strLabel[s_iMAX_STRING_SIZE];
        //    StringCchPrintf( strLabel, s_iMAX_STRING_SIZE, L"Maxinum number of samples: %d", g_nMaxSamples ); 
        //    g_SampleUI.GetStatic( IDC_MAX_NUM_SAMPLES_STATIC )->SetText( strLabel );
        //    break;
        //}

        //case IDC_TOGGLE_SHADOWS:
        //{
        //    // Toggle shadows static button:
        //    g_bDisplayShadows = (bool) ((CDXUTCheckBox*)pControl)->GetChecked();

        //    WCHAR strLabel[s_iMAX_STRING_SIZE];
        //    if ( g_bDisplayShadows  )
        //        StringCchPrintf( strLabel, s_iMAX_STRING_SIZE, L"Toggle self-occlusion shadows rendering: ON " );
        //    else
        //        StringCchPrintf( strLabel, s_iMAX_STRING_SIZE, L"Toggle self-occlusion shadows rendering: OFF " );
        //    g_SampleUI.GetCheckBox( IDC_TOGGLE_SHADOWS )->SetText( strLabel );
        //    break; 
        //}

        //case IDC_TECHNIQUE_COMBO_BOX:
        //{
        //    DXUTComboBoxItem* pSelectedItem = ((CDXUTComboBox*)pControl)->GetSelectedItem();
        //    if ( pSelectedItem )
        //        g_nCurrentTechniqueID = (int)(INT_PTR) pSelectedItem->pData;

        //    break;
        //}  

        //case IDC_TOGGLE_SPECULAR:
        //{
        //    // Toggle specular static button:
        //    g_bAddSpecular = (bool) ((CDXUTCheckBox*)pControl)->GetChecked();

        //    WCHAR strLabel[s_iMAX_STRING_SIZE];
        //    if ( g_bAddSpecular ) 
        //        StringCchPrintf( strLabel, s_iMAX_STRING_SIZE, L"Toggle specular: ON" );
        //    else
        //        StringCchPrintf( strLabel, s_iMAX_STRING_SIZE, L"Toggle specular: OFF" );
        //    g_SampleUI.GetCheckBox( IDC_TOGGLE_SPECULAR )->SetText( strLabel );
        //    break; 
        //}

        //case IDC_SELECT_TEXTURES_COMBOBOX:
        //{
        //    DXUTComboBoxItem* pSelectedItem = ((CDXUTComboBox*)pControl)->GetSelectedItem();
        //    if( pSelectedItem )
        //        SwitchPOMTextures( pSelectedItem );
        //    break; 
        //}

        //case IDC_RELOAD_BUTTON:
        //{
        //    HRESULT hr;
        //    V( LoadEffectFile() );
        //    break;
        //} 
    }  
}  


//--------------------------------------------------------------------------------------
// Release D3D9 resources created in the OnD3D9ResetDevice callback 
//--------------------------------------------------------------------------------------
void CALLBACK OnLostDevice( void* pUserContext )
{
   g_DialogResourceManager.OnLostDevice();
   g_SettingsDlg.OnLostDevice();
   CDXUTDirectionWidget::StaticOnLostDevice();

   if ( g_pFont ) g_pFont->OnLostDevice();
   if ( g_pEffect ) g_pEffect->OnLostDevice();
   SAFE_RELEASE( g_pSprite );
}  


//--------------------------------------------------------------------------------------
// Release D3D9 resources created in the OnD3D9CreateDevice callback 
//--------------------------------------------------------------------------------------
void CALLBACK OnDestroyDevice( void* pUserContext )
{
   g_DialogResourceManager.OnDestroyDevice();
   g_SettingsDlg.OnDestroyDevice();
   CDXUTDirectionWidget::StaticOnDestroyDevice();
   
   // Release the memory used for meshes / fonts / effect / textures:
   SAFE_RELEASE( g_pEffect );
   SAFE_RELEASE( g_pFont );
   SAFE_RELEASE( g_pMesh );

   pDepthBuffer->Release();
   pDepthBuffer = NULL;

   pBackBuffer->Release();
   pBackBuffer = NULL;

	

   SAFE_RELEASE( g_depthTexture );
   free( g_depthTexture );
   SAFE_RELEASE( g_noiseTexture );
   free( g_noiseTexture );
   SAFE_RELEASE( pDepthBuffer );



} 




