/*
 * Copyright 2011-2015 Branimir Karadzic. All rights reserved.
 * License: http://www.opensource.org/licenses/BSD-2-Clause
 */

#ifndef BGFX_GLCONTEXT_EGL_H_HEADER_GUARD
#define BGFX_GLCONTEXT_EGL_H_HEADER_GUARD

#if BGFX_USE_EGL

#include <EGL/egl.h>

namespace bgfx { namespace gl
{
	struct SwapChainGL;

	struct GlContext
	{
		GlContext()
			: m_current(NULL)
			, m_context(NULL)
			, m_display(NULL)
			, m_surface(NULL)
		{
		}

		void create(uint32_t _width, uint32_t _height);
		void destroy();
		void resize(uint32_t _width, uint32_t _height, uint32_t _flags);

		static bool isSwapChainSupported();
		SwapChainGL* createSwapChain(void* _nwh);
		void destroySwapChain(SwapChainGL*  _swapChain);
		void swap(SwapChainGL* _swapChain = NULL);
		void makeCurrent(SwapChainGL* _swapChain = NULL);

		void import();

		bool isValid() const
		{
			return NULL != m_context;
		}

		void* m_eglLibrary;
		SwapChainGL* m_current;
		EGLConfig  m_config;
		EGLContext m_context;
		EGLDisplay m_display;
		EGLSurface m_surface;
	};
} /* namespace gl */ } // namespace bgfx

#endif // BGFX_USE_EGL

#endif // BGFX_GLCONTEXT_EGL_H_HEADER_GUARD