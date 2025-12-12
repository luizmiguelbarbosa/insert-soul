#version 330

uniform sampler2D texture0; 
in vec2 fragTexCoord; 
out vec4 finalColor; 

uniform vec2 resolution; // Mantida, mas não utilizada
uniform float time;      // Mantida, mas utilizada apenas para flicker sutil

// --- FUNÇÕES AUXILIARES ---

// Ajusta o contraste da cor (Fator 1.8 - Alto)
vec3 applyContrast(vec3 color, float contrast) {
    return ((color - 0.5) * contrast + 0.5);
}

// Aumenta a saturação da cor (Fator 1.5 - Forte)
vec3 applySaturation(vec3 color, float saturation) {
    float luma = dot(color, vec3(0.2126, 0.7152, 0.0722));
    return mix(vec3(luma), color, saturation);
}

// Função shadowMask REMOVIDA

// --- FUNÇÃO PRINCIPAL ---

void main() {
    vec2 uv = fragTexCoord;
    vec2 originalUV = uv; 

    // 1. Curvatura CRT (Barrel Distortion) - REMOVIDA
    // 2. Verificação de Limites/Bugs - REMOVIDA
    
    // 3. Amostragem da Cor
    vec3 color = texture(texture0, uv).rgb;

    // 4. Ajuste de Saturação (CORES FORTES)
    color = applySaturation(color, 1.5);

    // 5. Ajuste de Contraste (ALTO CONTRASTE)
    color = applyContrast(color, 1.8); 

    // 6. Scanlines (Linhas horizontais) - REMOVIDA
    
    // 7. Efeito de Máscara de Fósforo (Shadow Mask) - REMOVIDO
    
    // 8. Vinheta (Suave, para dar um toque final)
    float vignette = 1.0 - (length(originalUV - 0.5) * 0.75);
    vignette = pow(vignette, 0.8);
    color *= vignette;
    
    // 9. Ruído Analógico Sutil (Flicker) - Mantido, se desejar um toque de movimento
    float flicker = sin(time * 15.0) * 0.01 + 0.01;
    color += flicker * vec3(0.005);
    
    finalColor = vec4(color, 1.0);
}