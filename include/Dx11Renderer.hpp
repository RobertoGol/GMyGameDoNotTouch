#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "include/Renderer.hpp"

#include <d3d11.h>
#include <dxgi.h>
#include <cstddef>
#include <wrl/client.h>

struct GLFWwindow;

namespace bunker {

struct Dx11Renderer {
    bool Initialize(GLFWwindow* window, int width, int height);
    void Shutdown();
    void BeginFrame(float r, float g, float b, float a);
    void EndFrame();
    void Resize(int width, int height);
    void RenderTerrain(
        const World& world,
        WeatherAnomaly weather = WeatherAnomaly::Clear,
        float weatherIntensity = 0.0f,
        float sessionTimeMinutes = 0.0f);
    bool LoadTextureFromBA2(const unsigned char* ddsData, std::size_t dataSize);
    bool LoadDDSTextureFromMemory(const unsigned char* ddsData, std::size_t dataSize);

    ID3D11Device* Device() const { return device.Get(); }
    ID3D11DeviceContext* Context() const { return context.Get(); }

private:
    bool CreateRenderTarget();
    bool CreateTerrainPipeline();
    bool CreateFallbackTerrainTexture();

    Microsoft::WRL::ComPtr<ID3D11Device> device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
    Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> terrainVertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> terrainPixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> terrainInputLayout;
    Microsoft::WRL::ComPtr<ID3D11Buffer> terrainVertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> terrainTextureView;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> terrainSamplerState;
    unsigned terrainVertexCount = 0;
    int viewportWidth = 1;
    int viewportHeight = 1;
    bool imguiBackendReady = false;
};

}  // namespace bunker
