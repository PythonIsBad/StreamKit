uniform sampler2D texture;
uniform vec2 screen_resolution;

const float colorDown = 4.0, DCTDown = 2.0;
const int downsamplingSize = 4;
const float PI = 3.14159;

float getCbMidColor(vec2 pixelCoord) // downsampled
{
	float color = 0.0;
	vec4 coloredPixelColor;
	for(int x = int(pixelCoord.x) * downsamplingSize; x < int(pixelCoord.x) * downsamplingSize + downsamplingSize; x++)
	{
		for(int y = int(pixelCoord.y) * downsamplingSize; y < int(pixelCoord.y) * downsamplingSize + downsamplingSize; y++)
		{
			coloredPixelColor = texture2D(texture, vec2(float(x), float(y)) / screen_resolution) * 256.0;
			color += clamp(-0.14713 * coloredPixelColor.r + -0.28886 * coloredPixelColor.g + 0.436 * coloredPixelColor.b + 128.0, 0.0, 255.0);
		}
	}
	color = color / float(downsamplingSize * downsamplingSize) / 256.0;
	return color;
}

float getCrMidColor(vec2 pixelCoord) // downsampled
{
	float color = 0.0;
	vec4 coloredPixelColor;
	for(int x = int(pixelCoord.x) * downsamplingSize; x < int(pixelCoord.x) * downsamplingSize + downsamplingSize; x++)
	{
		for(int y = int(pixelCoord.y) * downsamplingSize; y < int(pixelCoord.y) * downsamplingSize + downsamplingSize; y++)
		{
			coloredPixelColor = texture2D(texture, vec2(float(x), float(y)) / screen_resolution) * 256.0;
			color += clamp(0.615 * coloredPixelColor.r + -0.51499 * coloredPixelColor.g + -0.10001 * coloredPixelColor.b + 128.0, 0.0, 255.0);
		}
	}
	color = color / float(downsamplingSize * downsamplingSize) / 256.0;
	return color;
}

float getCbDCTColor(vec2 pixelCoord) // downsampled
{
	vec2 squareCoord, localPixelCoord;
	float pixelColor = 0.0, tmpDouble = 0.0, color = 0.0;
	
	squareCoord = vec2(pixelCoord.x - float(int(pixelCoord.x) % 8), pixelCoord.y - float(int(pixelCoord.y) % 8));
	localPixelCoord = vec2(float(int(pixelCoord.x) % 8), float(int(pixelCoord.y) % 8));
	
	for (int u = 0; u < 8; u++)
		for (int v = 0; v < 8; v++)
			tmpDouble = tmpDouble +
			(getCbMidColor(vec2(squareCoord.x + float(u), squareCoord.y + float(v))) - 0.5) *
			cos(int((2.0 * float(u) + 1.0)) * localPixelCoord.x * PI / 16.0) *
			cos(int((2.0 * float(v) + 1.0)) * localPixelCoord.y * PI / 16.0);
	color = 0.25 * (int(localPixelCoord.x) == 0 ? 1.0 / sqrt(2.0) : 1.0) * (int(localPixelCoord.y) == 0 ? 1.0 / sqrt(2.0) : 1.0) * tmpDouble / DCTDown;
	if(color < 0.0)
	{
		if(color > -0.01)
			color = 0.0;
		else
			color += float(int(color) + 1);
	}
	if(color > 1.0)
	{
		if(color < 1.01)
			color = 1.0;
		else
			color -= float(int(color));
	}
	return color;
}

float getCrDCTColor(vec2 pixelCoord) // downsampled
{
	vec2 squareCoord, localPixelCoord;
	float pixelColor = 0.0, tmpDouble = 0.0, color = 0.0;
	
	squareCoord = vec2(pixelCoord.x - float(int(pixelCoord.x) % 8), pixelCoord.y - float(int(pixelCoord.y) % 8));
	localPixelCoord = vec2(float(int(pixelCoord.x) % 8), float(int(pixelCoord.y) % 8));
	
	for (int u = 0; u < 8; u++)
		for (int v = 0; v < 8; v++)
			tmpDouble = tmpDouble +
			(getCrMidColor(vec2(squareCoord.x + float(u), squareCoord.y + float(v))) - 0.5) *
			cos(int((2.0 * float(u) + 1.0)) * localPixelCoord.x * PI / 16.0) *
			cos(int((2.0 * float(v) + 1.0)) * localPixelCoord.y * PI / 16.0);
	color = 0.25 * (int(localPixelCoord.x) == 0 ? 1.0 / sqrt(2.0) : 1.0) * (int(localPixelCoord.y) == 0 ? 1.0 / sqrt(2.0) : 1.0) * tmpDouble / DCTDown;
	if(color < 0.0)
	{
		if(color > -0.01)
			color = 0.0;
		else
			color += float(int(color) + 1);
	}
	if(color > 1.0)
	{
		if(color < 1.01)
			color = 1.0;
		else
			color -= float(int(color));
	}
	return color;
}

float getYColor(vec2 pixelCoord)
{
	vec4 rawPixelColor = texture2D(texture, pixelCoord / screen_resolution) * 256.0;
	return clamp(0.299 * rawPixelColor.r + 0.587 * rawPixelColor.g + 0.114 * rawPixelColor.b, 0.0, 255.0) / 256.0;
}

float getYDCTColor(vec2 pixelCoord)
{
	vec2 squareCoord, localPixelCoord;
	squareCoord = vec2(pixelCoord.x - float(int(pixelCoord.x) % 8), pixelCoord.y - float(int(pixelCoord.y) % 8));
	localPixelCoord = vec2(float(int(pixelCoord.x) % 8), float(int(pixelCoord.y) % 8));
	float tmpDouble = 0.0, color = 0.0;
	for (int u = 0; u < 8; u++)
		for (int v = 0; v < 8; v++)
			tmpDouble = tmpDouble +
			(getYColor(vec2(squareCoord.x + float(u), squareCoord.y + float(v))) - 0.5) / colorDown *
			cos(int((2.0 * float(u) + 1.0)) * localPixelCoord.x * PI / 16.0) *
			cos(int((2.0 * float(v) + 1.0)) * localPixelCoord.y * PI / 16.0);
	color = 0.25 * (int(localPixelCoord.x) == 0 ? 1.0 / sqrt(2.0) : 1.0) * (int(localPixelCoord.y) == 0 ? 1.0 / sqrt(2.0) : 1.0) * tmpDouble / DCTDown;
	if(color < 0.0)
	{
		if(color > -0.01)
			color = 0.0;
		else
			color += float(int(color) + 1);
	}
	if(color > 1.0)
	{
		if(color < 1.01)
			color = 1.0;
		else
			color -= float(int(color));
	}
	return color;
}

void main()
{
	vec2 pixelCoord = vec2(gl_FragCoord.x, screen_resolution.y - gl_FragCoord.y);
	vec4 pixelColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	if(pixelCoord.x < screen_resolution.x / downsamplingSize && pixelCoord.y < screen_resolution.y / downsamplingSize)
	{
		pixelColor.g = getCbDCTColor(pixelCoord);
		pixelColor.b = getCrDCTColor(pixelCoord);
	}
	pixelColor.r = getYDCTColor(pixelCoord);
	
	gl_FragColor = pixelColor;
}