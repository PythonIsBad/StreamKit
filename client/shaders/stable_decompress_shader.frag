uniform sampler2D texture;
uniform vec2 screen_resolution;

const float colorDown = 4.0, DCTDown = 2.0;
const int downsamplingSize = 4;
const float PI = 3.14159;

float getCbColor(vec2 pixelCoord)
{
	vec2 squareCoord, localPixelCoord;
	float color = 0.0, tmpDouble = 0.0;
	
	squareCoord = vec2(pixelCoord.x - float(int(pixelCoord.x) % 8), pixelCoord.y - float(int(pixelCoord.y) % 8));
	localPixelCoord = vec2(float(int(pixelCoord.x) % 8), float(int(pixelCoord.y) % 8));
	
	for (int u = 0; u < 8; u++)
		for (int v = 0; v < 8; v++)
			tmpDouble = tmpDouble +
			(u == 0 ? 1.0 / sqrt(2.0) : 1.0) *
			(v == 0 ? 1.0 / sqrt(2.0) : 1.0) *
			//texture2D(texture, pixelCoord / screen_resolution / downsamplingSize).g
			(texture2D(texture,
			vec2(float(squareCoord.x + u),
			float(squareCoord.y + v))
			/ screen_resolution).g * 256.0 - 128.0) * DCTDown *
			cos((2.0 * localPixelCoord.x + 1.0) * float(u) * PI / 16.0) *
			cos((2.0 * localPixelCoord.y + 1.0) * float(v) * PI / 16.0);
	color = (0.25 * tmpDouble + 128.0) / 256.0;
	
	return color;
}

float getCrColor(vec2 pixelCoord)
{
	vec2 squareCoord, localPixelCoord;
	float color = 0.0, tmpDouble = 0.0;
	
	squareCoord = vec2(pixelCoord.x - float(int(pixelCoord.x) % 8), pixelCoord.y - float(int(pixelCoord.y) % 8));
	localPixelCoord = vec2(float(int(pixelCoord.x) % 8), float(int(pixelCoord.y) % 8));
	
	for (int u = 0; u < 8; u++)
		for (int v = 0; v < 8; v++)
			tmpDouble = tmpDouble +
			(u == 0 ? 1.0 / sqrt(2.0) : 1.0) *
			(v == 0 ? 1.0 / sqrt(2.0) : 1.0) *
			//texture2D(texture, pixelCoord / screen_resolution / downsamplingSize).g
			(texture2D(texture,
			vec2(float(squareCoord.x + u),
			float(squareCoord.y + v))
			/ screen_resolution).b * 256.0 - 128.0) * DCTDown *
			cos((2.0 * localPixelCoord.x + 1.0) * float(u) * PI / 16.0) *
			cos((2.0 * localPixelCoord.y + 1.0) * float(v) * PI / 16.0);
	color = (0.25 * tmpDouble + 128.0) / 256.0;
	
	return color;
}

vec4 getColor(vec2 pixelCoord)
{
	vec2 squareCoord, localPixelCoord;
	vec4 pixelColor, rawPixelColor = vec4(0.0, 0.0, 0.0, 0.0);
	float tmpDouble = 0.0;
	
	squareCoord = vec2(pixelCoord.x - float(int(pixelCoord.x) % 8), pixelCoord.y - float(int(pixelCoord.y) % 8));
	localPixelCoord = vec2(float(int(pixelCoord.x) % 8), float(int(pixelCoord.y) % 8));
	
	for (int u = 0; u < 8; u++)
		for (int v = 0; v < 8; v++)
			tmpDouble = tmpDouble +
			(u == 0 ? 1.0 / sqrt(2.0) : 1.0) *
			(v == 0 ? 1.0 / sqrt(2.0) : 1.0) *
			(texture2D(texture,
			vec2(float(squareCoord.x + u),
			float(squareCoord.y + v))
			/ screen_resolution).r * 256.0 - 128.0) * DCTDown *
			cos((2.0 * localPixelCoord.x + 1.0) * float(u) * PI / 16.0) *
			cos((2.0 * localPixelCoord.y + 1.0) * float(v) * PI / 16.0);
			
	rawPixelColor.r = (0.25 * tmpDouble * colorDown + 128.0) / 256.0;
	rawPixelColor.g = getCbColor(pixelCoord / downsamplingSize);
	rawPixelColor.b = getCrColor(pixelCoord / downsamplingSize);
	rawPixelColor *= 256.0;
	
	pixelColor.r = clamp(rawPixelColor.r + 1.13983 * (rawPixelColor.g - 128.0), 0.0, 255.0) / 256.0;
	pixelColor.g = clamp(rawPixelColor.r - 0.39465 * (rawPixelColor.b - 128.0) + -0.5806 * (rawPixelColor.g - 128.0), 0.0, 255.0) / 256.0;
	pixelColor.b = clamp(rawPixelColor.r + 2.03211 * (rawPixelColor.b - 128.0) + 0.0 * (rawPixelColor.g - 128.0), 0.0, 255.0) / 256.0;
	pixelColor.a = 1.0;
	
	return pixelColor;
}

void main()
{
	if(int(gl_FragCoord.x) % 8 == 0 && int(screen_resolution.y - gl_FragCoord.y) % 8 == 0)
	{
		vec4 midColor = getColor(vec2(gl_FragCoord.x, screen_resolution.y - gl_FragCoord.y));
		midColor += getColor(vec2(gl_FragCoord.x + 1, screen_resolution.y - gl_FragCoord.y));
		midColor += getColor(vec2(gl_FragCoord.x - 1, screen_resolution.y - gl_FragCoord.y));
		midColor += getColor(vec2(gl_FragCoord.x, screen_resolution.y - gl_FragCoord.y + 1));
		midColor += getColor(vec2(gl_FragCoord.x, screen_resolution.y - gl_FragCoord.y - 1));
		midColor /= 5.0;
		gl_FragColor = midColor;
	}
	else
		gl_FragColor = getColor(vec2(gl_FragCoord.x, screen_resolution.y - gl_FragCoord.y));
}