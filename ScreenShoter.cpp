#include "ScreenShoter.h"
cv::Mat DXScreenShoter11::ExtractBitmap(ID3D11Texture2D* d3dtex, ID3D11Device* pDevice)
{
	D3D11_TEXTURE2D_DESC desc;
	ID3D11Texture2D* pNewTexture = NULL;

	D3D11_TEXTURE2D_DESC description;

	d3dtex->GetDesc(&desc);
	d3dtex->GetDesc(&description);

	description.BindFlags = 0;
	description.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	description.Usage = D3D11_USAGE_STAGING;
	description.MiscFlags = 0;

	pDevice->CreateTexture2D(&description, NULL, &pNewTexture);
	ctx->CopyResource(pNewTexture, d3dtex);

	D3D11_MAPPED_SUBRESOURCE resource;
	ctx->Map(pNewTexture, D3D11CalcSubresource(0, 0, 0), D3D11_MAP_READ, 0, &resource);

	// Copy from texture to bitmap buffer.
	cv::Mat Texture = cv::Mat(desc.Height, desc.Width, CV_8UC4);
	for (int i = 0; i < desc.Height; ++i)
	{
		memcpy(Texture.data + desc.Width * i * 4, (uchar*)resource.pData + desc.Width * i * 4, desc.Width * 4);
	}

	ctx->Unmap(pNewTexture, 0);
	pNewTexture->Release();

	return Texture;
}
void DXScreenShoter11::Init()
{


	D3D_FEATURE_LEVEL d3dFeatLvl;
	IDXGIFactory1* pFactory;
	IDXGIAdapter1* pAdapter;
	IDXGIOutput* pOutput;
	IDXGIOutput1* pOutput1;


	hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)(&pFactory));

	hr = pFactory->EnumAdapters1(0, &pAdapter);

	hr = pAdapter->EnumOutputs(0, &pOutput);

	hr = pOutput->QueryInterface(__uuidof(IDXGIOutput1), reinterpret_cast<void**>(&pOutput1));

	hr = D3D11CreateDevice(pAdapter, D3D_DRIVER_TYPE_UNKNOWN,
		0, 0, 0, 0,
		D3D11_SDK_VERSION,
		&pDevice,
		&d3dFeatLvl,
		&pImmediateContext);
	hr = pOutput1->DuplicateOutput(pDevice, &pDeskDupl);

	pDevice->GetImmediateContext(&ctx);

};
cv::Mat DXScreenShoter11::Take()
{
	DXGI_OUTDUPL_FRAME_INFO FrameInfo;

	// Get new frame.
	hr = pDeskDupl->AcquireNextFrame(0, &FrameInfo, &DesktopResource);
	if (FAILED(hr))
	{
		return cv::Mat(1080, 1920, CV_8UC4);
	}

	hr = DesktopResource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&AcquiredDesktopImage));
	DesktopResource->Release();

	cv::Mat hBmp =
		ExtractBitmap(AcquiredDesktopImage, pDevice);
	hr = pDeskDupl->ReleaseFrame();
	return hBmp;
}
