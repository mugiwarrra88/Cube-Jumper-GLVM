#version 410 core
out vec4 fragColor;

in VS_OUT {
	vec3 fragmentPosition;
	vec3 normal;
	vec2 textureCoords;
	vec4 fragmentPositionDirectionalLightSpace[2];
	vec4 fragmentPositionSpotLightSpace[2];
} fs_in;

struct Material {
    vec3      ambient;
	sampler2D diffuse;
	sampler2D specular;
    float     shininess;
}; 

uniform Material material;
uniform float time; // Time uniform for animation
uniform float numberToShow; // Number to display (0-999)

// Function to get bit from digit bitmap (5x7 pixels)
float getBit(int bitmap[7], int x, int y) {
    if (x < 0 || x >= 5 || y < 0 || y >= 7) return 0.0;
    return float((bitmap[y] >> (4 - x)) & 1);
}

// Enhanced digit bitmaps (5x7 pixels each for better quality)
void getDigitBitmap(int digit, out int bitmap[7]) {
    if (digit == 0) {
        bitmap[0] = 0x0E; // 01110
        bitmap[1] = 0x11; // 10001
        bitmap[2] = 0x11; // 10001
        bitmap[3] = 0x11; // 10001
        bitmap[4] = 0x11; // 10001
        bitmap[5] = 0x11; // 10001
        bitmap[6] = 0x0E; // 01110
    } else if (digit == 1) {
        bitmap[0] = 0x04; // 00100
        bitmap[1] = 0x0C; // 01100
        bitmap[2] = 0x04; // 00100
        bitmap[3] = 0x04; // 00100
        bitmap[4] = 0x04; // 00100
        bitmap[5] = 0x04; // 00100
        bitmap[6] = 0x1F; // 11111
    } else if (digit == 2) {
        bitmap[0] = 0x0E; // 01110
        bitmap[1] = 0x11; // 10001
        bitmap[2] = 0x01; // 00001
        bitmap[3] = 0x02; // 00010
        bitmap[4] = 0x04; // 00100
        bitmap[5] = 0x08; // 01000
        bitmap[6] = 0x1F; // 11111
    } else if (digit == 3) {
        bitmap[0] = 0x0E; // 01110
        bitmap[1] = 0x11; // 10001
        bitmap[2] = 0x01; // 00001
        bitmap[3] = 0x06; // 00110
        bitmap[4] = 0x01; // 00001
        bitmap[5] = 0x11; // 10001
        bitmap[6] = 0x0E; // 01110
    } else if (digit == 4) {
        bitmap[0] = 0x02; // 00010
        bitmap[1] = 0x06; // 00110
        bitmap[2] = 0x0A; // 01010
        bitmap[3] = 0x12; // 10010
        bitmap[4] = 0x1F; // 11111
        bitmap[5] = 0x02; // 00010
        bitmap[6] = 0x02; // 00010
    } else if (digit == 5) {
        bitmap[0] = 0x1F; // 11111
        bitmap[1] = 0x10; // 10000
        bitmap[2] = 0x1E; // 11110
        bitmap[3] = 0x01; // 00001
        bitmap[4] = 0x01; // 00001
        bitmap[5] = 0x11; // 10001
        bitmap[6] = 0x0E; // 01110
    } else if (digit == 6) {
        bitmap[0] = 0x06; // 00110
        bitmap[1] = 0x08; // 01000
        bitmap[2] = 0x10; // 10000
        bitmap[3] = 0x1E; // 11110
        bitmap[4] = 0x11; // 10001
        bitmap[5] = 0x11; // 10001
        bitmap[6] = 0x0E; // 01110
    } else if (digit == 7) {
        bitmap[0] = 0x1F; // 11111
        bitmap[1] = 0x01; // 00001
        bitmap[2] = 0x02; // 00010
        bitmap[3] = 0x04; // 00100
        bitmap[4] = 0x08; // 01000
        bitmap[5] = 0x08; // 01000
        bitmap[6] = 0x08; // 01000
    } else if (digit == 8) {
        bitmap[0] = 0x0E; // 01110
        bitmap[1] = 0x11; // 10001
        bitmap[2] = 0x11; // 10001
        bitmap[3] = 0x0E; // 01110
        bitmap[4] = 0x11; // 10001
        bitmap[5] = 0x11; // 10001
        bitmap[6] = 0x0E; // 01110
    } else if (digit == 9) {
        bitmap[0] = 0x0E; // 01110
        bitmap[1] = 0x11; // 10001
        bitmap[2] = 0x11; // 10001
        bitmap[3] = 0x0F; // 01111
        bitmap[4] = 0x01; // 00001
        bitmap[5] = 0x02; // 00010
        bitmap[6] = 0x0C; // 01100
    } else {
        // Default case - fill with zeros
        for (int i = 0; i < 7; i++) {
            bitmap[i] = 0x00;
        }
    }
}

// Function to render a single digit at specified position with smoothing
float renderDigit(vec2 uv, vec2 pos, float scale, int digit) {
    vec2 digitUV = (uv - pos) / scale;
    
    if (digitUV.x < 0.0 || digitUV.x > 1.0 || digitUV.y < 0.0 || digitUV.y > 1.0) {
        return 0.0;
    }
    
    // Scale to 5x7 grid
    float fx = digitUV.x * 5.0;
    float fy = (1.0 - digitUV.y) * 7.0;
    
    int x = int(fx);
    int y = int(fy);
    
    // Get fractional parts for smoothing
    float fracX = fract(fx);
    float fracY = fract(fy);
    
    int bitmap[7];
    getDigitBitmap(digit, bitmap);
    
    // Sample with bilinear filtering for smoother edges
    float p00 = getBit(bitmap, x, y);
    float p10 = getBit(bitmap, x + 1, y);
    float p01 = getBit(bitmap, x, y + 1);
    float p11 = getBit(bitmap, x + 1, y + 1);
    
    float p0 = mix(p00, p10, fracX);
    float p1 = mix(p01, p11, fracX);
    
    return mix(p0, p1, fracY);
}

// Function to render a number (up to 3 digits) with improved spacing and effects
float renderNumber(vec2 uv, vec2 pos, float scale, float number) {
    int num = int(number);
    float result = 0.0;
    
    // Add glow effect around digits
    vec2 glowUV = uv;
    float glow = 0.0;
    
    if (num >= 100) {
        int hundreds = num / 100;
        float digit = renderDigit(uv, pos + vec2(0.0, 0.0), scale, hundreds);
        result += digit;
        
        // Add glow
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                vec2 offset = vec2(float(dx), float(dy)) * scale * 0.1;
                glow += renderDigit(uv, pos + vec2(0.0, 0.0) + offset, scale, hundreds) * 0.3;
            }
        }
    }
    
    if (num >= 10) {
        int tens = (num % 100) / 10;
        float digit = renderDigit(uv, pos + vec2(scale * 1.4, 0.0), scale, tens);
        result += digit;
        
        // Add glow
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                vec2 offset = vec2(float(dx), float(dy)) * scale * 0.1;
                glow += renderDigit(uv, pos + vec2(scale * 1.4, 0.0) + offset, scale, tens) * 0.3;
            }
        }
    }
    
    int ones = num % 10;
    float digit = renderDigit(uv, pos + vec2(scale * 2.8, 0.0), scale, ones);
    result += digit;
    
    // Add glow for ones digit
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            vec2 offset = vec2(float(dx), float(dy)) * scale * 0.1;
            glow += renderDigit(uv, pos + vec2(scale * 2.8, 0.0) + offset, scale, ones) * 0.3;
        }
    }
    
    return clamp(result + glow * 0.5, 0.0, 1.0);
}

void main()
{
	// Sample the main texture
	vec4 textureColor = texture(material.diffuse, fs_in.textureCoords);
	
	// Apply material ambient color for tinting
	vec3 baseColor = textureColor.rgb * material.ambient;
	
	// Add animated glow effect based on texture coordinates
	vec2 center = vec2(0.5, 0.5);
	float distanceFromCenter = distance(fs_in.textureCoords, center);
	
	// Create pulsing glow effect
	float pulse = sin(time * 3.0) * 0.5 + 0.5; // Oscillates between 0 and 1
	float glow = 1.0 - distanceFromCenter;
	glow = pow(glow, 2.0) * pulse * 0.3; // Soft glow from center
	
	// Add subtle color shifting based on position
	vec3 colorShift = vec3(
		sin(time + fs_in.textureCoords.x * 6.28) * 0.1,
		sin(time * 1.3 + fs_in.textureCoords.y * 6.28) * 0.1,
		sin(time * 0.7 + distanceFromCenter * 6.28) * 0.1
	);
	
	// Add fresnel-like rim lighting
	vec3 viewDir = normalize(-fs_in.fragmentPosition);
	float fresnel = 1.0 - abs(dot(normalize(fs_in.normal), viewDir));
	fresnel = pow(fresnel, 2.0);	vec3 rimLight = material.ambient * fresnel * 0.4;
		// Render number on texture with enhanced visuals
	float numberMask = renderNumber(fs_in.textureCoords, vec2(0.35, 0.4), 0.06, numberToShow);
		// Create animated color for the number
	vec3 numberColor = vec3(
		0.8 + 0.2 * sin(time * 2.0),           // Red component with pulsing
		0.9 + 0.1 * sin(time * 3.0 + 1.0),    // Green component 
		0.1 + 0.4 * sin(time * 1.5 + 2.0)     // Blue component for golden effect
	);
		// Add outline effect
	float outline = 0.0;
	float outlineWidth = 0.008;
	for (int dx = -2; dx <= 2; dx++) {
		for (int dy = -2; dy <= 2; dy++) {
			if (dx == 0 && dy == 0) continue;
			vec2 offset = vec2(float(dx), float(dy)) * outlineWidth;
			outline += renderNumber(fs_in.textureCoords + offset, vec2(0.35, 0.4), 0.06, numberToShow);
		}
	}
	outline = clamp(outline, 0.0, 1.0) * (1.0 - numberMask); // Only show outline where there's no digit
	
	// Combine effects
	vec3 finalColor = baseColor + (baseColor * glow) + colorShift + rimLight;
	
	// Add number with glow and outline
	finalColor = mix(finalColor, numberColor, numberMask);
	finalColor = mix(finalColor, vec3(0.0, 0.0, 0.0), outline * 0.8); // Dark outline
	
	// Add slight metallic shine
	finalColor += vec3(0.1) * pow(fresnel, 4.0);
	
	fragColor = vec4(finalColor, textureColor.a);
}


