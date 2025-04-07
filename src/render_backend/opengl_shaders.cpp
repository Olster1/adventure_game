static char *quadVertexShader = 
"#version 330\n"
//per vertex variables
"in vec3 vertex;"
"in vec2 texUV;	"

//per instanced variables
"in vec3 pos;"
"in vec4 uvAtlas;"
"in vec4 color;"
"in vec2 scale;"
"in float samplerIndex;"
"in uint aoMask;"

//uniform variables
"uniform mat4 projection;"

//outgoing variables
"out vec4 color_frag;"
"out vec2 uv_frag;"
"out float texture_array_index;"

"void main() {"
    "vec3 p = vertex;"

    "p.x *= scale.x;"
    "p.y *= scale.y;"

    "p += pos;"

    "gl_Position = projection * vec4(p, 1.0f);"
    "color_frag = color;"
    "texture_array_index = samplerIndex;"

    "uv_frag = vec2(mix(uvAtlas.x, uvAtlas.z, texUV.x), mix(uvAtlas.y, uvAtlas.w, texUV.y));"
"}";

static char *quadAoMaskVertexShader = 
"#version 330\n"
//per vertex variables
"in vec3 vertex;"
"in vec2 texUV;	"

//per instanced variables
"in vec3 pos;"
"in vec4 uvAtlas;"
"in vec4 color;"
"in vec2 scale;"
"in float samplerIndex;"
"in uint aoMask;"

//uniform variables
"uniform mat4 projection;"

//outgoing variables
"out vec4 color_frag;"
"out vec2 uv_frag;"
"out float texture_array_index;"
"out float AOValue;"

"float aoFactors[4] = float[4](1, 0.6, 0.5, 0.3);"

"void main() {"
    "vec3 p = vertex;"

    "p.x *= scale.x;"
    "p.y *= scale.y;"

    "p += pos;"

    "gl_Position = projection * vec4(p, 1.0f);"
    "color_frag = color;"
    "texture_array_index = samplerIndex;"

    "uint aoIndex = uint(3);"
    "uint mask = aoMask >> uint(gl_VertexID*2);"
    "AOValue = aoFactors[mask & aoIndex];"

    "uv_frag = vec2(mix(uvAtlas.x, uvAtlas.z, texUV.x), mix(uvAtlas.y, uvAtlas.w, texUV.y));"
"}";

static char *pixelArtAoMaskFragShader = 
"#version 330\n"
"in vec4 color_frag;" 
"in vec2 uv_frag; "
"in float AOValue;"
"uniform sampler2D diffuse;"
"out vec4 color;"
"void main() {"
    "vec2 size = textureSize(diffuse, 0);"
    "vec2 uv = uv_frag * size;"
    "vec2 duv = fwidth(uv);"
    "uv = floor(uv) + 0.5 + clamp(((fract(uv) - 0.5 + duv)/duv), 0.0, 1.0);"
    "uv /= size;"
    "vec4 sample = texture(diffuse, uv);"
    "sample = vec4(AOValue*sample.xyz, sample.w);"
   
    "color = sample*color_frag;"
"}";


static char *rectOutlineVertexShader = 
"#version 330\n"
//per vertex variables
"in vec3 vertex;"
"in vec2 texUV;	"

//per instanced variables
"in vec3 pos;"
"in vec4 uvAtlas;"
"in vec4 color;"
"in vec2 scale;"
"in float samplerIndex;"

//uniform variables
"uniform mat4 projection;"

//outgoing variables
"out vec4 color_frag;"
"out vec2 uv_frag;"
"out float texture_array_index;"
"out vec2 scale_world_space;"

"void main() {"
    "vec3 p = vertex;"

    "p.x *= scale.x;"
    "p.y *= scale.y;"

    "p += pos;"

    "gl_Position = projection * vec4(p, 1.0f);"
    "color_frag = color;"
    "texture_array_index = samplerIndex;"

    "uv_frag = texUV;"
    "scale_world_space = scale;"
"}";

static char *lineVertexShader = 
"#version 330\n"
//per vertex variables
"in vec3 vertex;"
"in vec2 texUV;	"

//per instanced variables
"vec3 pos1;"
"vec3 pos2;"
"vec4 color1;"

//uniform variables
"uniform mat4 projection;"

//outgoing variables
"out vec4 color_frag;"

"void main() {"
    "vec3 pos = vertex.x * pos1 + vertex.y * pos2;"
    "gl_Position = projection * vec4(pos, 1.0f);"
    "color_frag = color1;"
"}";

static char *rectOutlineFragShader = 
"#version 330\n"
"in vec4 color_frag;" 
"in vec2 uv_frag;"
"in vec2 scale_world_space;"
"out vec4 colorOut;"
"void main() {"
    "float outline_width = 2;    "
    
    "float alpha_value = 0.0f;"

    "vec4 color = color_frag;"

    "float xAt = uv_frag.x*scale_world_space.x;"
    "float yAt = uv_frag.y*scale_world_space.y;"
    "if(xAt < outline_width || (scale_world_space.x - xAt) < outline_width || yAt < outline_width || (scale_world_space.y - yAt) < outline_width) {"
        "alpha_value = 1.0f;"
    "}"
    "color.a = alpha_value;"

    "colorOut = color;"
"}";


static char *quadTextureFragShader = 
"#version 330\n"
"in vec4 color_frag;" 
"in vec2 uv_frag; "
"uniform sampler2D diffuse;"
"out vec4 color;"
"void main() {"
    "vec4 diffSample = texture(diffuse, uv_frag);"
    "color = diffSample*color_frag;"
"}";

static char *sdfFragShader = 
"#version 330\n"
"in vec4 color_frag;" 
"in vec2 uv_frag; "
"uniform sampler2D diffuse;"
"out vec4 colorOut;"
"void main() {"
    "float smoothing = 0.2f;"
    "float boldness = 0.3f;"
    "vec4 sample = texture(diffuse, uv_frag); "

    "float distance = sample.a;"

    "float alpha = smoothstep(1.0f - boldness, (1.0f - boldness) + smoothing, distance);"

    "vec4 color = alpha * color_frag;"

    "color.xyz /= color.a;"

    "colorOut = color;"
"}";

static char *pixelArtFragShader = 
"#version 330\n"
"in vec4 color_frag;" 
"in vec2 uv_frag; "
"uniform sampler2D diffuse;"
"out vec4 color;"
"void main() {"
    "vec2 size = textureSize(diffuse, 0);"
    "vec2 uv = uv_frag * size;"
    "vec2 duv = fwidth(uv);"
    "uv = floor(uv) + 0.5 + clamp(((fract(uv) - 0.5 + duv)/duv), 0.0, 1.0);"
    "uv /= size;"
    "vec4 sample = texture(diffuse, uv);"
   
    "color = sample*color_frag;"
"}";

static char *lineFragShader = 
"#version 330\n"
"in vec4 color_frag;" 
"out vec4 color;"
"void main() {"
    "color = color_frag;"
"}";