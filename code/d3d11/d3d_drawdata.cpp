#include "d3d_common.h"
#include "d3d_state.h"
#include "d3d_drawdata.h"
#include "d3d_shaders.h"
#include "d3d_image.h"

static void CreateVertexLayoutAndShader( 
    const char* shaderName, 
    const D3D11_INPUT_ELEMENT_DESC* elements,
    size_t numElements,
    ID3D11VertexShader** vshader,
    ID3D11InputLayout** layout )
{
    ID3DBlob* vsByteCode = nullptr;
    *vshader = CompileVertexShader( shaderName, &vsByteCode );

    HRESULT hr = g_pDevice->CreateInputLayout(
        elements,
        (UINT) numElements,
        vsByteCode->GetBufferPointer(),
        vsByteCode->GetBufferSize(),
        layout );
    if ( FAILED( hr ) ) {
        ri.Error( ERR_FATAL, "Failed to create input layout: 0x%08X", hr );
    }

    SAFE_RELEASE( vsByteCode );
}

void InitQuadRenderData( d3dQuadRenderData_t* qrd ) 
{
    Com_Memset( qrd, 0, sizeof( d3dQuadRenderData_t ) );

    D3D11_INPUT_ELEMENT_DESC elements[] = { 
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    CreateVertexLayoutAndShader( "fsq_vs", elements, _countof(elements), &qrd->vertexShader, &qrd->inputLayout );
    qrd->pixelShader = CompilePixelShader( "fsq_ps" );

    static const USHORT indices[] = 
    {
	    1, 2, 0,
	    2, 3, 0
    };

    qrd->indexBuffer = QD3D::CreateImmutableBuffer( 
        g_pDevice, 
        D3D11_BIND_INDEX_BUFFER, 
        indices,
        sizeof( indices ) );
    if ( !qrd->indexBuffer ) {
        ri.Error( ERR_FATAL, "Could not create FSQ index buffer.\n" );
    }

    qrd->vertexBuffer = QD3D::CreateDynamicBuffer<d3dQuadRenderVertex_t>(
        g_pDevice, 
        D3D11_BIND_VERTEX_BUFFER,
        4);
    if ( !qrd->vertexBuffer ) {
        ri.Error( ERR_FATAL, "Could not create FSQ vertex buffer.\n" );
    }

    qrd->constantBuffer = QD3D::CreateDynamicBuffer<d3dQuadRenderConstantBuffer_t>( 
        g_pDevice, 
        D3D11_BIND_CONSTANT_BUFFER );
    if ( !qrd->constantBuffer ) {
        ri.Error( ERR_FATAL, "Could not create FSQ constant buffer.\n" );
    }
}

void DestroyQuadRenderData( d3dQuadRenderData_t* qrd )
{
    SAFE_RELEASE( qrd->inputLayout );
    SAFE_RELEASE( qrd->vertexShader );
    SAFE_RELEASE( qrd->pixelShader );
    SAFE_RELEASE( qrd->vertexBuffer );
    SAFE_RELEASE( qrd->indexBuffer );
    SAFE_RELEASE( qrd->constantBuffer );

    Com_Memset( qrd, 0, sizeof( *qrd ) );
}

void InitSkyBoxRenderData( d3dSkyBoxRenderData_t* rd ) 
{
    Com_Memset( rd, 0, sizeof( d3dSkyBoxRenderData_t ) );

    D3D11_INPUT_ELEMENT_DESC elements[] = { 
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    CreateVertexLayoutAndShader( "skybox_vs", elements, _countof(elements), &rd->vertexShader, &rd->inputLayout );
    rd->pixelShader = CompilePixelShader( "skybox_ps" );

    static const d3dSkyBoxVertex_t vertices[] = 
    {
        -1, -1, -1, 0, 0, 0,
         1, -1, -1, 0, 0, 0,
        -1,  1, -1, 0, 0, 0,
         1,  1, -1, 0, 0, 0,
        -1, -1,  1, 0, 0, 0,
         1, -1,  1, 0, 0, 0,
        -1,  1,  1, 0, 0, 0,
         1,  1,  1, 0, 0, 0,
    };

    static const USHORT indices[] = 
    {
        4, 0, 2, 4, 2, 6, // Right
        1, 5, 7, 1, 7, 3, // Left
        6, 2, 3, 6, 3, 7, // Back 
        5, 1, 0, 5, 0, 4, // Front
        5, 4, 6, 5, 6, 7, // Up     
        0, 1, 3, 0, 3, 2, // Down
    };

    rd->indexBuffer = QD3D::CreateImmutableBuffer( 
        g_pDevice, 
        D3D11_BIND_INDEX_BUFFER, 
        indices,
        sizeof( indices ) );
    if ( !rd->indexBuffer ) {
        ri.Error( ERR_FATAL, "Could not create SkyBox index buffer.\n" );
    }

    rd->vertexBuffer = QD3D::CreateImmutableBuffer(
        g_pDevice, 
        D3D11_BIND_VERTEX_BUFFER,
        vertices,
        sizeof(vertices) );
    if ( !rd->vertexBuffer ) {
        ri.Error( ERR_FATAL, "Could not create SkyBox vertex buffer.\n" );
    }

    rd->vsConstantBuffer = QD3D::CreateDynamicBuffer<d3dSkyBoxVSConstantBuffer_t>( 
        g_pDevice, 
        D3D11_BIND_CONSTANT_BUFFER );
    if ( !rd->vsConstantBuffer ) {
        ri.Error( ERR_FATAL, "Could not create SkyBox VS constant buffer.\n" );
    }

    rd->psConstantBuffer = QD3D::CreateDynamicBuffer<d3dSkyBoxPSConstantBuffer_t>( 
        g_pDevice, 
        D3D11_BIND_CONSTANT_BUFFER );
    if ( !rd->psConstantBuffer ) {
        ri.Error( ERR_FATAL, "Could not create SkyBox PS constant buffer.\n" );
    }
}

void DestroySkyBoxRenderData( d3dSkyBoxRenderData_t* rd )
{
    SAFE_RELEASE( rd->inputLayout );
    SAFE_RELEASE( rd->vertexShader );
    SAFE_RELEASE( rd->pixelShader );
    SAFE_RELEASE( rd->vertexBuffer );
    SAFE_RELEASE( rd->indexBuffer );
    SAFE_RELEASE( rd->vsConstantBuffer );
    SAFE_RELEASE( rd->psConstantBuffer );

    Com_Memset( rd, 0, sizeof( *rd ) );
}

void InitGenericStageRenderData( d3dGenericStageRenderData_t* rd )
{
    Com_Memset( rd, 0, sizeof( *rd ) );
    
    static_assert( NUM_TEXTURE_BUNDLES == 2, "Need to munge this code for anything but 2 texture bundles" );

    D3D11_INPUT_ELEMENT_DESC elementsST[] = { 
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,  0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,     1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R8G8B8A8_UNORM,   2, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    D3D11_INPUT_ELEMENT_DESC elementsMT[] = { 
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,  0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,     1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT,     2, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R8G8B8A8_UNORM,   3, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    CreateVertexLayoutAndShader( "genericmt_vs", elementsMT, _countof(elementsMT), &rd->vertexShaderMT, &rd->inputLayoutMT );
    CreateVertexLayoutAndShader( "genericst_vs", elementsST, _countof(elementsST), &rd->vertexShaderST, &rd->inputLayoutST );
    rd->pixelShaderMT = CompilePixelShader( "genericmt_ps" );
    rd->pixelShaderST = CompilePixelShader( "genericst_ps" );
}

void DestroyGenericStageRenderData( d3dGenericStageRenderData_t* rd )
{
    SAFE_RELEASE( rd->inputLayoutST );
    SAFE_RELEASE( rd->vertexShaderST );
    SAFE_RELEASE( rd->pixelShaderST );
    SAFE_RELEASE( rd->inputLayoutMT );
    SAFE_RELEASE( rd->vertexShaderMT );
    SAFE_RELEASE( rd->pixelShaderMT );

    Com_Memset( rd, 0, sizeof( *rd ) );
}

void InitViewRenderData( d3dViewRenderData_t* vrd )
{
    Com_Memset( vrd, 0, sizeof( d3dViewRenderData_t ) );

    vrd->vsConstantBuffer = QD3D::CreateDynamicBuffer<d3dViewVSConstantBuffer_t>( 
        g_pDevice, 
        D3D11_BIND_CONSTANT_BUFFER );
    if ( !vrd->vsConstantBuffer ) {
        ri.Error( ERR_FATAL, "Could not create view constant buffer.\n" );
    }

    vrd->psConstantBuffer = QD3D::CreateDynamicBuffer<d3dViewPSConstantBuffer_t>( 
        g_pDevice, 
        D3D11_BIND_CONSTANT_BUFFER );
    if ( !vrd->psConstantBuffer ) {
        ri.Error( ERR_FATAL, "Could not create view constant buffer.\n" );
    }
}

void DestroyViewRenderData( d3dViewRenderData_t* vrd )
{
    SAFE_RELEASE( vrd->vsConstantBuffer );
    SAFE_RELEASE( vrd->psConstantBuffer );

    Com_Memset( vrd, 0, sizeof( d3dViewRenderData_t ) );
}

void InitRasterStates( d3dRasterStates_t* rs )
{
    Com_Memset( rs, 0, sizeof( d3dRasterStates_t ) );

    D3D11_RASTERIZER_DESC rd;
    ZeroMemory( &rd, sizeof( rd ) );
    rd.FrontCounterClockwise = TRUE;

    for ( int cullMode = 0; cullMode < CULLMODE_COUNT; ++cullMode )
    {
        rd.CullMode = (D3D11_CULL_MODE)( cullMode + 1 );

        for ( int rasterMode = 0; rasterMode < RASTERIZERSTATE_COUNT; ++rasterMode )
        {
            if ( rasterMode & RASTERIZERSTATE_FLAG_POLY_OFFSET ) {
                rd.DepthBias = r_offsetFactor->value;
                rd.SlopeScaledDepthBias = r_offsetUnits->value;
                rd.DepthBiasClamp = 1;
            } else {
                rd.DepthBias = 0;
                rd.SlopeScaledDepthBias = 0;
                rd.DepthBiasClamp = 1;
            }

            if ( rasterMode & RASTERIZERSTATE_FLAG_POLY_OUTLINE ) {
                rd.FillMode = D3D11_FILL_WIREFRAME;
            } else {
                rd.FillMode = D3D11_FILL_SOLID;
            }

            g_pDevice->CreateRasterizerState( &rd, &rs->states[cullMode][rasterMode] );
        }
    }
}

void DestroyRasterStates( d3dRasterStates_t* rs )
{
    for ( int cullMode = 0; cullMode < CULLMODE_COUNT; ++cullMode )
    {
        for ( int rasterMode = 0; rasterMode < RASTERIZERSTATE_COUNT; ++rasterMode )
        {
            SAFE_RELEASE( rs->states[cullMode][rasterMode] );
        }
    }

    Com_Memset( rs, 0, sizeof( d3dRasterStates_t ) );
}

static ID3D11DepthStencilState* CreateDepthStencilStateFromMask( unsigned long mask )
{
    ID3D11DepthStencilState* state = nullptr;

    D3D11_DEPTH_STENCIL_DESC dsd;
    ZeroMemory( &dsd, sizeof( dsd ) );

    if ( mask & DEPTHSTATE_FLAG_TEST ) {
        dsd.DepthEnable = TRUE;
    }
    if ( mask & DEPTHSTATE_FLAG_MASK ) {
        dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    }
    if ( mask & DEPTHSTATE_FLAG_EQUAL ) {
        dsd.DepthFunc = D3D11_COMPARISON_EQUAL;
    } else {
        dsd.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    }

    g_pDevice->CreateDepthStencilState( &dsd, &state );
    if ( !state ) {
        ri.Error( ERR_FATAL, "Failed to create DepthStencilState of mask %x\n", mask );
    }

    return state;
}

void InitDepthStates( d3dDepthStates_t* ds )
{
    Com_Memset( ds, 0, sizeof( d3dDepthStates_t ) );

    for ( int i = 0; i < _countof( ds->states ); ++i )
    {
        ds->states[i] = CreateDepthStencilStateFromMask( i );
    }
}

void DestroyDepthStates( d3dDepthStates_t* ds )
{
    for ( int i = 0; i < _countof( ds->states ); ++i )
    {
        SAFE_RELEASE( ds->states[i] );
    }

    Com_Memset( ds, 0, sizeof( d3dDepthStates_t ) );
}

void InitBlendStates( d3dBlendStates_t* bs )
{
    Com_Memset( bs, 0, sizeof( d3dBlendStates_t ) );

    // 
    // No blending
    //
    D3D11_BLEND_DESC bsd;
    ZeroMemory( &bsd, sizeof( bsd ) );
    bsd.RenderTarget[0].BlendEnable = FALSE;
    bsd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    g_pDevice->CreateBlendState( &bsd, &bs->opaque );

    //
    // Blend-mode matrix
    //
    bsd.RenderTarget[0].BlendEnable = TRUE;
    bsd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    bsd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    for ( int src = 0; src < BLENDSTATE_SRC_COUNT; ++src )
    {
        for ( int dst = 0; dst < BLENDSTATE_DST_COUNT; ++dst )
        {
            int qSrc = src + 1;
            int qDst = (dst + 1) << 4;
            bsd.RenderTarget[0].SrcBlend = GetSrcBlendConstant( qSrc );
            bsd.RenderTarget[0].DestBlend = GetDestBlendConstant( qDst );
            bsd.RenderTarget[0].SrcBlendAlpha = GetSrcBlendAlphaConstant( qSrc );
            bsd.RenderTarget[0].DestBlendAlpha = GetDestBlendAlphaConstant( qDst );
            g_pDevice->CreateBlendState( &bsd, &bs->states[src][dst] );
        }
    }
}

void DestroyBlendStates( d3dBlendStates_t* bs )
{
    SAFE_RELEASE( bs->opaque );

    for ( int src = 0; src < BLENDSTATE_SRC_COUNT; ++src )
    {
        for ( int dst = 0; dst < BLENDSTATE_DST_COUNT; ++dst )
        {
            SAFE_RELEASE( bs->states[src][dst] );
        }
    }

    Com_Memset( bs, 0, sizeof( d3dBlendStates_t ) );
}

static ID3D11Buffer* CreateTessIndexBuffer()
{
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

    // @pjb: todo: glIndex_t is apparently 32 bit :(
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = (UINT)sizeof(glIndex_t) * SHADER_MAX_INDEXES;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	
	ID3D11Buffer* buffer;
	g_pDevice->CreateBuffer(&bd, NULL, &buffer);
    if ( !buffer ) {
        ri.Error( ERR_FATAL, "Could not create tess index buffer.\n" );
    }
    
    return buffer;
}

static ID3D11Buffer* CreateTessVertexBuffer( size_t size )
{
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = (UINT)size * SHADER_MAX_VERTEXES;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	
	ID3D11Buffer* buffer;
	g_pDevice->CreateBuffer(&bd, NULL, &buffer);
    if ( !buffer ) {
        ri.Error( ERR_FATAL, "Could not create tess vertex buffer.\n" );
    }
    
    return buffer;
}

void InitTessBuffers( d3dTessBuffers_t* tess )
{
    Com_Memset( tess, 0, sizeof( *tess ) );

    // Index buffer
    tess->indexes = CreateTessIndexBuffer();

    // Vertex buffer
    tess->xyz = CreateTessVertexBuffer( sizeof(vec4_t) );

    //
    // Now set up the stages
    //
    for ( int stage = 0; stage < MAX_SHADER_STAGES; ++stage )
    {
        d3dTessStageBuffers_t* stageBufs = &tess->stages[stage];

        // Color buffer
        stageBufs->colors = CreateTessVertexBuffer( sizeof(color4ub_t) );

        // Texcoord buffers
        for ( int tex = 0; tex < NUM_TEXTURE_BUNDLES; ++tex )
        {
            // Color buffer
            stageBufs->texCoords[tex] = CreateTessVertexBuffer( sizeof(vec2_t) );
        }
    }

    //
    // Now set up dlight projection buffers
    //
    for ( int l = 0; l < MAX_DLIGHTS; ++l )
    {
        d3dTessLightProjBuffers_t* buffers = &tess->dlights[l];

        buffers->indexes = CreateTessIndexBuffer();
        buffers->colors = CreateTessVertexBuffer( sizeof(byte) * 4 );
        buffers->texCoords = CreateTessVertexBuffer( sizeof(float) * 2 );
    }

    //
    // Now set up fog buffers
    //
    tess->fog.colors = CreateTessVertexBuffer( sizeof(color4ub_t) );
    tess->fog.texCoords = CreateTessVertexBuffer( sizeof(vec2_t) );
}

void DestroyTessBuffers( d3dTessBuffers_t* tess )
{
    SAFE_RELEASE( tess->indexes );
    SAFE_RELEASE( tess->xyz );

    for ( int stage = 0; stage < MAX_SHADER_STAGES; ++stage )
    {
        SAFE_RELEASE( tess->stages[stage].colors );
        for ( int tex = 0; tex < NUM_TEXTURE_BUNDLES; ++tex )
            SAFE_RELEASE( tess->stages[stage].texCoords[tex] );
    }

    for ( int l = 0; l < MAX_DLIGHTS; ++l )
    {
        SAFE_RELEASE( tess->dlights[l].indexes );
        SAFE_RELEASE( tess->dlights[l].colors );
        SAFE_RELEASE( tess->dlights[l].texCoords );
    }

    SAFE_RELEASE( tess->fog.colors );
    SAFE_RELEASE( tess->fog.texCoords );

    Com_Memset( tess, 0, sizeof( *tess ) );
}

