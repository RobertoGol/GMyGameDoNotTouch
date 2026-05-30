#include "Dx11Renderer.hpp"

#ifndef NOMINMAX
#define NOMINMAX
#endif

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <backends/imgui_impl_dx11.h>
#include <backends/imgui_impl_glfw.h>
#include <d3dcompiler.h>
#include <imgui.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <vector>

#pragma comment(lib, "d3dcompiler.lib")

namespace bunker {
namespace {

struct TerrainVertex {
    float x;
    float y;
    float z;
    float r;
    float g;
    float b;
    float a;
    float u;
    float v;
};

constexpr char kTerrainShader[] = R"(
struct VSIn {
    float3 position : POSITION;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
};

struct PSIn {
    float4 position : SV_POSITION;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
};

Texture2D terrainTexture : register(t0);
SamplerState terrainSampler : register(s0);

PSIn VSMain(VSIn input) {
    PSIn output;
    output.position = float4(input.position, 1.0);
    output.color = input.color;
    output.uv = input.uv;
    return output;
}

float4 PSMain(PSIn input) : SV_Target {
    return terrainTexture.Sample(terrainSampler, input.uv) * input.color;
}
)";

constexpr std::uint32_t MakeFourCC(char a, char b, char c, char d) {
    return static_cast<std::uint32_t>(a) |
        (static_cast<std::uint32_t>(b) << 8) |
        (static_cast<std::uint32_t>(c) << 16) |
        (static_cast<std::uint32_t>(d) << 24);
}

std::uint32_t ReadU32(const unsigned char* data, std::size_t offset) {
    std::uint32_t value = 0;
    std::memcpy(&value, data + offset, sizeof(value));
    return value;
}

DXGI_FORMAT FormatFromDDS(const unsigned char* data, std::size_t dataSize, std::size_t& dataOffset, unsigned& blockBytes) {
    const std::uint32_t fourCC = ReadU32(data, 84);
    dataOffset = 128;
    blockBytes = 16;
    if (fourCC == MakeFourCC('D', 'X', 'T', '1')) {
        blockBytes = 8;
        return DXGI_FORMAT_BC1_UNORM;
    }
    if (fourCC == MakeFourCC('D', 'X', 'T', '3')) {
        return DXGI_FORMAT_BC2_UNORM;
    }
    if (fourCC == MakeFourCC('D', 'X', 'T', '5')) {
        return DXGI_FORMAT_BC3_UNORM;
    }
    if (fourCC == MakeFourCC('D', 'X', '1', '0') && dataSize >= 148) {
        dataOffset = 148;
        const auto format = static_cast<DXGI_FORMAT>(ReadU32(data, 128));
        if (format == DXGI_FORMAT_BC1_UNORM || format == DXGI_FORMAT_BC1_UNORM_SRGB) {
            blockBytes = 8;
            return format;
        }
        if (format == DXGI_FORMAT_BC2_UNORM || format == DXGI_FORMAT_BC2_UNORM_SRGB ||
            format == DXGI_FORMAT_BC3_UNORM || format == DXGI_FORMAT_BC3_UNORM_SRGB ||
            format == DXGI_FORMAT_BC7_UNORM || format == DXGI_FORMAT_BC7_UNORM_SRGB) {
            return format;
        }
    }
    return DXGI_FORMAT_UNKNOWN;
}

float NormalizeCoord(float value, float minValue, float maxValue) {
    const float span = std::max(1.0f, maxValue - minValue);
    return ((value - minValue) / span) * 2.0f - 1.0f;
}

}  // namespace

bool Dx11Renderer::Initialize(GLFWwindow* window, int width, int height) {
    viewportWidth = std::max(1, width);
    viewportHeight = std::max(1, height);

    DXGI_SWAP_CHAIN_DESC swapDesc{};
    swapDesc.BufferCount = 2;
    swapDesc.BufferDesc.Width = static_cast<UINT>(viewportWidth);
    swapDesc.BufferDesc.Height = static_cast<UINT>(viewportHeight);
    swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapDesc.OutputWindow = glfwGetWin32Window(window);
    swapDesc.SampleDesc.Count = 1;
    swapDesc.Windowed = TRUE;
    swapDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    const std::array<D3D_FEATURE_LEVEL, 3> levels = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };

    D3D_FEATURE_LEVEL selectedLevel = D3D_FEATURE_LEVEL_11_0;
    if (FAILED(D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            0,
            levels.data(),
            static_cast<UINT>(levels.size()),
            D3D11_SDK_VERSION,
            &swapDesc,
            swapChain.GetAddressOf(),
            device.GetAddressOf(),
            &selectedLevel,
            context.GetAddressOf()))) {
        return false;
    }

    if (!CreateRenderTarget() || !CreateTerrainPipeline() || !CreateFallbackTerrainTexture()) {
        return false;
    }

    ImGui_ImplGlfw_InitForOther(window, true);
    ImGui_ImplDX11_Init(device.Get(), context.Get());
    imguiBackendReady = true;
    return true;
}

void Dx11Renderer::Shutdown() {
    if (imguiBackendReady) {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        imguiBackendReady = false;
    }

    terrainVertexBuffer.Reset();
    terrainSamplerState.Reset();
    terrainTextureView.Reset();
    terrainInputLayout.Reset();
    terrainPixelShader.Reset();
    terrainVertexShader.Reset();
    renderTargetView.Reset();
    swapChain.Reset();
    context.Reset();
    device.Reset();
}

void Dx11Renderer::BeginFrame(float r, float g, float b, float a) {
    const float clearColor[4] = {r, g, b, a};
    context->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), nullptr);
    context->ClearRenderTargetView(renderTargetView.Get(), clearColor);
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void Dx11Renderer::EndFrame() {
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
    swapChain->Present(1, 0);
}

void Dx11Renderer::Resize(int width, int height) {
    if (!swapChain) {
        return;
    }

    viewportWidth = std::max(1, width);
    viewportHeight = std::max(1, height);
    renderTargetView.Reset();
    swapChain->ResizeBuffers(0, static_cast<UINT>(viewportWidth), static_cast<UINT>(viewportHeight), DXGI_FORMAT_UNKNOWN, 0);
    CreateRenderTarget();
}

void Dx11Renderer::RenderTerrain(
    const World& world,
    WeatherAnomaly weather,
    float weatherIntensity,
    float sessionTimeMinutes) {
    float minX = -128.0f;
    float maxX = 128.0f;
    float minZ = -128.0f;
    float maxZ = 128.0f;

    for (const auto& object : world.objects) {
        minX = std::min(minX, object.x - object.width);
        maxX = std::max(maxX, object.x + object.width);
        minZ = std::min(minZ, object.y - object.depth);
        maxZ = std::max(maxZ, object.y + object.depth);
    }

    std::vector<TerrainVertex> vertices;
    const float step = 2.0f;
    const float stormBlend = weather == WeatherAnomaly::EtherFog
        ? std::clamp(weatherIntensity, 0.0f, 1.0f)
        : 0.0f;
    for (float z = std::floor(minZ / step) * step; z < maxZ; z += step) {
        for (float x = std::floor(minX / step) * step; x < maxX; x += step) {
            const float nx1 = NormalizeCoord(x, minX, maxX);
            const float nz1 = NormalizeCoord(z, minZ, maxZ);
            const float nx2 = NormalizeCoord(x + step, minX, maxX);
            const float nz2 = NormalizeCoord(z + step, minZ, maxZ);
            float ny = world.GetWorldHeightAt(x, z) * 0.1f;
            float nyX = world.GetWorldHeightAt(x + step, z) * 0.1f;
            float nyZ = world.GetWorldHeightAt(x, z + step) * 0.1f;
            if (stormBlend > 0.0f) {
                const float stormPhase = sessionTimeMinutes * 48.0f;
                ny += std::sin((x * 0.19f) + (z * 0.13f) + stormPhase) * 0.08f * stormBlend;
                nyX += std::sin(((x + step) * 0.19f) + (z * 0.13f) + stormPhase) * 0.08f * stormBlend;
                nyZ += std::sin((x * 0.19f) + ((z + step) * 0.13f) + stormPhase) * 0.08f * stormBlend;
            }
            ny = std::clamp(ny, -0.35f, 0.65f);
            nyX = std::clamp(nyX, -0.35f, 0.65f);
            nyZ = std::clamp(nyZ, -0.35f, 0.65f);

            const float r = 0.18f + (0.90f - 0.18f) * stormBlend;
            const float g = 0.34f + (0.16f - 0.34f) * stormBlend;
            const float b = 0.24f + (0.12f - 0.24f) * stormBlend;
            constexpr float a = 1.0f;
            const float u1 = (x - minX) / std::max(1.0f, maxX - minX);
            const float v1 = (z - minZ) / std::max(1.0f, maxZ - minZ);
            const float u2 = (x + step - minX) / std::max(1.0f, maxX - minX);
            const float v2 = (z + step - minZ) / std::max(1.0f, maxZ - minZ);

            vertices.push_back({nx1, ny, nz1, r, g, b, a, u1, v1});
            vertices.push_back({nx2, nyX, nz1, r, g, b, a, u2, v1});
            vertices.push_back({nx1, ny, nz1, r, g, b, a, u1, v1});
            vertices.push_back({nx1, nyZ, nz2, r, g, b, a, u1, v2});
        }
    }

    D3D11_BUFFER_DESC bufferDesc{};
    bufferDesc.ByteWidth = static_cast<UINT>(vertices.size() * sizeof(TerrainVertex));
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    D3D11_SUBRESOURCE_DATA vertexData{};
    vertexData.pSysMem = vertices.data();
    if (FAILED(device->CreateBuffer(&bufferDesc, &vertexData, terrainVertexBuffer.ReleaseAndGetAddressOf()))) {
        terrainVertexCount = 0;
        return;
    }

    terrainVertexCount = static_cast<unsigned>(vertices.size());
    const UINT stride = sizeof(TerrainVertex);
    const UINT offset = 0;
    context->IASetInputLayout(terrainInputLayout.Get());
    context->IASetVertexBuffers(0, 1, terrainVertexBuffer.GetAddressOf(), &stride, &offset);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    context->VSSetShader(terrainVertexShader.Get(), nullptr, 0);
    context->PSSetShader(terrainPixelShader.Get(), nullptr, 0);
    if (terrainTextureView && terrainSamplerState) {
        context->PSSetShaderResources(0, 1, terrainTextureView.GetAddressOf());
        context->PSSetSamplers(0, 1, terrainSamplerState.GetAddressOf());
    }
    context->Draw(terrainVertexCount, 0);
}

bool Dx11Renderer::LoadTextureFromBA2(const unsigned char* ddsData, std::size_t dataSize) {
    return LoadDDSTextureFromMemory(ddsData, dataSize);
}

bool Dx11Renderer::LoadDDSTextureFromMemory(const unsigned char* ddsData, std::size_t dataSize) {
    if (!device || ddsData == nullptr || dataSize < 128 || std::memcmp(ddsData, "DDS ", 4) != 0 || ReadU32(ddsData, 4) != 124) {
        return false;
    }

    std::size_t dataOffset = 0;
    unsigned blockBytes = 0;
    const DXGI_FORMAT format = FormatFromDDS(ddsData, dataSize, dataOffset, blockBytes);
    if (format == DXGI_FORMAT_UNKNOWN || dataOffset >= dataSize) {
        return false;
    }

    const UINT width = ReadU32(ddsData, 16);
    const UINT height = ReadU32(ddsData, 12);
    const UINT mipLevels = std::max<UINT>(1, ReadU32(ddsData, 28));
    if (width == 0 || height == 0) {
        return false;
    }

    std::vector<D3D11_SUBRESOURCE_DATA> subresources;
    subresources.reserve(mipLevels);
    std::size_t offset = dataOffset;
    for (UINT mip = 0; mip < mipLevels; ++mip) {
        const UINT mipWidth = std::max<UINT>(1, width >> mip);
        const UINT mipHeight = std::max<UINT>(1, height >> mip);
        const UINT blocksWide = std::max<UINT>(1, (mipWidth + 3) / 4);
        const UINT blocksHigh = std::max<UINT>(1, (mipHeight + 3) / 4);
        const UINT rowPitch = blocksWide * blockBytes;
        const std::size_t sliceBytes = static_cast<std::size_t>(rowPitch) * blocksHigh;
        if (offset + sliceBytes > dataSize) {
            break;
        }
        subresources.push_back({ddsData + offset, rowPitch, static_cast<UINT>(sliceBytes)});
        offset += sliceBytes;
    }
    if (subresources.empty()) {
        return false;
    }

    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = static_cast<UINT>(subresources.size());
    desc.ArraySize = 1;
    desc.Format = format;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
    if (FAILED(device->CreateTexture2D(&desc, subresources.data(), texture.GetAddressOf()))) {
        return false;
    }
    if (FAILED(device->CreateShaderResourceView(texture.Get(), nullptr, terrainTextureView.ReleaseAndGetAddressOf()))) {
        return false;
    }

    D3D11_SAMPLER_DESC samplerDesc{};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    return SUCCEEDED(device->CreateSamplerState(&samplerDesc, terrainSamplerState.ReleaseAndGetAddressOf()));
}

bool Dx11Renderer::CreateFallbackTerrainTexture() {
    const std::uint32_t whitePixel = 0xffffffffu;
    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = 1;
    desc.Height = 1;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA initData{};
    initData.pSysMem = &whitePixel;
    initData.SysMemPitch = sizeof(whitePixel);

    Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
    if (FAILED(device->CreateTexture2D(&desc, &initData, texture.GetAddressOf())) ||
        FAILED(device->CreateShaderResourceView(texture.Get(), nullptr, terrainTextureView.ReleaseAndGetAddressOf()))) {
        return false;
    }

    D3D11_SAMPLER_DESC samplerDesc{};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    return SUCCEEDED(device->CreateSamplerState(&samplerDesc, terrainSamplerState.ReleaseAndGetAddressOf()));
}

bool Dx11Renderer::CreateRenderTarget() {
    Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
    if (FAILED(swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf())))) {
        return false;
    }
    if (FAILED(device->CreateRenderTargetView(backBuffer.Get(), nullptr, renderTargetView.GetAddressOf()))) {
        return false;
    }

    D3D11_VIEWPORT viewport{};
    viewport.Width = static_cast<float>(viewportWidth);
    viewport.Height = static_cast<float>(viewportHeight);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    context->RSSetViewports(1, &viewport);
    return true;
}

bool Dx11Renderer::CreateTerrainPipeline() {
    Microsoft::WRL::ComPtr<ID3DBlob> vertexBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> pixelBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

    if (FAILED(D3DCompile(kTerrainShader, sizeof(kTerrainShader), nullptr, nullptr, nullptr, "VSMain", "vs_4_0", 0, 0, vertexBlob.GetAddressOf(), errorBlob.GetAddressOf())) ||
        FAILED(D3DCompile(kTerrainShader, sizeof(kTerrainShader), nullptr, nullptr, nullptr, "PSMain", "ps_4_0", 0, 0, pixelBlob.GetAddressOf(), errorBlob.ReleaseAndGetAddressOf()))) {
        return false;
    }

    if (FAILED(device->CreateVertexShader(vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), nullptr, terrainVertexShader.GetAddressOf())) ||
        FAILED(device->CreatePixelShader(pixelBlob->GetBufferPointer(), pixelBlob->GetBufferSize(), nullptr, terrainPixelShader.GetAddressOf()))) {
        return false;
    }

    const D3D11_INPUT_ELEMENT_DESC layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    return SUCCEEDED(device->CreateInputLayout(layout, 3, vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), terrainInputLayout.GetAddressOf()));
}

}  // namespace bunker
