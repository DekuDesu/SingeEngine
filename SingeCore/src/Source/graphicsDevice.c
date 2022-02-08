#include "graphics/graphicsDevice.h"
#include "GL/glew.h"
#include <stdlib.h>
#include "graphics/textureDefinitions.h"

struct _comparisons Comparisons = {
	.Always = { "always", GL_ALWAYS },
	.Never = { "never", GL_NEVER },
	.Equal = { "equal", GL_EQUAL },
	.NotEqual = { "notEqual", GL_NOTEQUAL },
	.GreaterThan = { "greater", GL_GREATER },
	.LessThan = { "less", GL_LESS },
	.LessThanOrEqual = { "lessOrEqual", GL_LEQUAL },
	.GreaterThanOrEqual = { "greaterOrEqual", GL_GEQUAL }
};

static void EnableBlending(void);
static void DisableBlending(void);

static void EnableCulling(void);
static void DisableCulling(void);

static void EnableWritingToStencilBuffer(void);
static void DisableWritingToStencilBuffer(void);
static unsigned int GetStencilMask(void);
static void SetStencilMask(const unsigned int mask);
static void SetStencilFunction(const Comparison);
static void SetStencilFunctionFull(const Comparison, const unsigned int valueToCompareTo, const unsigned int mask);
static void ResetStencilFunction(void);

static void EnableDepthTesting(void);
static void DisableDepthTesting(void);
static void SetDepthTest(const Comparison);

static void ActivateTexture(const TextureType, const unsigned int textureHandle, const int uniformHandle, const unsigned int slot);
static unsigned int CreateTexture(const TextureType);
static void LoadTexture(const TextureType, TextureFormat, BufferFormat, Image, unsigned int offset);
static void ModifyTexture(const TextureType, TextureSetting, const TextureValue);
static void DeleteTexture(unsigned int handle);
static bool TryVerifyCleanup(void);

const struct _graphicsDeviceMethods GraphicsDevice = {
	.EnableBlending = &EnableBlending,
	.EnableCulling = &EnableCulling,
	.DisableBlending = &DisableBlending,
	.DisableCulling = &DisableCulling,
	.EnableStencilWriting = &DisableWritingToStencilBuffer,
	.DisableStencilWriting = DisableWritingToStencilBuffer,
	.SetStencilMask = &SetStencilMask,
	.GetStencilMask = GetStencilMask,
	.SetStencil = &SetStencilFunction,
	.ResetStencilFunction = &ResetStencilFunction,
	.SetStencilFull = &SetStencilFunctionFull,
	.EnableDepthTesting = &EnableDepthTesting,
	.DisableDepthTesting = &DisableDepthTesting,
	.SetDepthTest = &SetDepthTest,
	.ActivateTexture = &ActivateTexture,
	.CreateTexture = CreateTexture,
	.DeleteTexture = DeleteTexture,
	.TryVerifyCleanup = TryVerifyCleanup,
	.LoadTexture = LoadTexture,
	.ModifyTexture = ModifyTexture
};

bool blendingEnabled = false;
bool cullingEnabled = false;
bool writingToStencilBufferEnabled = false;
bool depthTestingEnabled = false;

unsigned int stencilMask = 0xFF;
unsigned int stencilComparisonFunction;
unsigned int stencilComparisonValue;
unsigned int stencilComparisonMask;
Comparison defaultStencilComparison = { "always", GL_ALWAYS };
unsigned int defaultStencilComparisonValue = 1;
unsigned int defaultStencilComparisonMask = 0xFF;
unsigned int depthComparison;

unsigned int nextTexture = 0;
unsigned int maxTextures = GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS;

// texture instance counts
size_t activeTextures = 0;


static void EnableDepthTesting(void)
{
	if (depthTestingEnabled is false)
	{
		glEnable(GL_DEPTH_TEST);
		depthTestingEnabled = true;
	}
}

static void DisableDepthTesting(void)
{
	if (depthTestingEnabled)
	{
		glDisable(GL_DEPTH_TEST);
		depthTestingEnabled = false;
	}
}

static void EnableBlending(void)
{
	if (blendingEnabled is false)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		blendingEnabled = true;
	}
}

static void DisableBlending(void)
{
	if (blendingEnabled)
	{
		glDisable(GL_BLEND);
		blendingEnabled = false;
	}
}

static void EnableCulling(void)
{
	if (cullingEnabled is false)
	{
		glEnable(GL_CULL_FACE);
		cullingEnabled = true;
	}
}

static void DisableCulling(void)
{
	if (cullingEnabled)
	{
		glDisable(GL_CULL_FACE);

		cullingEnabled = false;
	}
}

static void EnableWritingToStencilBuffer(void)
{
	if (writingToStencilBufferEnabled is false)
	{
		SetStencilMask(stencilMask);

		writingToStencilBufferEnabled = true;
	}
}

static void DisableWritingToStencilBuffer(void)
{
	if (writingToStencilBufferEnabled)
	{
		SetStencilMask(0x00);

		writingToStencilBufferEnabled = false;
	}
}

static unsigned int GetStencilMask(void)
{
	return stencilMask;
}

static void SetStencilMask(const unsigned int mask)
{
	stencilMask = mask;
}

static void SetStencilFunction(const Comparison comparison)
{
	if (comparison.Value.AsUInt isnt stencilComparisonFunction || stencilComparisonValue isnt 1 || stencilComparisonMask isnt 0xFF)
	{
		glStencilFunc(comparison.Value.AsUInt, 1, 0xFF);

		stencilComparisonFunction = comparison.Value.AsUInt;

		stencilComparisonValue = 1;

		stencilMask = 0xFF;
	}
}

static void SetStencilFunctionFull(const Comparison comparison, const unsigned int valueToCompareTo, const unsigned int mask)
{
	if (comparison.Value.AsUInt isnt stencilComparisonFunction || valueToCompareTo isnt stencilComparisonValue || mask != stencilComparisonMask)
	{
		glStencilFunc(comparison.Value.AsUInt, valueToCompareTo, mask);

		stencilComparisonFunction = comparison.Value.AsUInt;

		stencilComparisonValue = valueToCompareTo;

		stencilComparisonMask = mask;
	}
}

static void ResetStencilFunction(void)
{
	SetStencilFunctionFull(defaultStencilComparison, defaultStencilComparisonValue, defaultStencilComparisonMask);
}

static void SetDepthTest(const Comparison comparison)
{
	if (depthComparison isnt comparison.Value.AsUInt)
	{
		glDepthFunc(comparison.Value.AsUInt);
		depthComparison = comparison.Value.AsUInt;
	}
}

static void ActivateTexture(const TextureType textureType, const unsigned int textureHandle, const int uniformHandle, const unsigned int slot)
{
	unsigned int type = textureType.Value.AsUInt;
	glEnable(type);
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(type, textureHandle);
	glUniform1i(uniformHandle, slot);
	glDisable(type);
}

static unsigned int CreateTexture(TextureType type)
{
	unsigned int handle;
	glGenTextures(1, &handle);

	glBindTexture(type.Value.AsUInt, handle);

	// keep track of how many textures we create
	++(activeTextures);

	return handle;
}

static void DeleteTexture(unsigned int handle)
{
	glDeleteTextures(1, &handle);

	// keep track of how many textures we destroy
	--(activeTextures);
}

static void LoadTexture(TextureType type, TextureFormat colorFormat, BufferFormat pixelFormat, Image image, unsigned int offset)
{
	glTexImage2D(type.Value.AsUInt + offset, 0, colorFormat, image->Width, image->Height, 0, colorFormat, pixelFormat, image->Pixels);
}

static void ModifyTexture(TextureType type, TextureSetting setting, const TextureValue value)
{
	glTexParameteri(type.Value.AsUInt, setting, value.Value.AsInt);
}

static bool TryVerifyCleanup(void)
{
	bool result = true;

	// verify that all textures were destroyed
	fprintf(stderr, "Orphaned Textures: %lli"NEWLINE, activeTextures);

	result &= activeTextures is 0;

	return result;
}
