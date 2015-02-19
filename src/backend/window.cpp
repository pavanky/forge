/*******************************************************
 * Copyright (c) 2014, ArrayFire
 * All rights reserved.
 *
 * This file is distributed under 3-clause BSD license.
 * The complete license agreement can be obtained at:
 * http://arrayfire.com/licenses/BSD-3-Clause
 ********************************************************/

// Parts of this code sourced from SnopyDogy
// https://gist.github.com/SnopyDogy/a9a22497a893ec86aa3e

#include <window.hpp>
#include <common.hpp>
#include <err_common.hpp>

#include <stdexcept>
#include <iostream>
#include <cstring>
#include <cstdio>

namespace backend
{
    static int g_uiWindowCounter = 0; // Window Counter

    static void error_callback(int error, const char* description)
    {
        fputs(description, stderr);
        AFGFX_ERROR("Error in GLFW", AFGFX_ERR_GL_ERROR);
    }

    static void key_callback(GLFWwindow* wind, int key, int scancode, int action, int mods)
    {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(wind, GL_TRUE);
        }
    }

    template<typename T>
    afgfx_window createWindow(const unsigned disp_w, const unsigned disp_h, const char *title,
                              const afgfx_color_mode mode)
    {
        // save current active context info so we can restore it later!
        //afgfx_window previous = current;

        // create new window data:
        afgfx_window newWindow = new afgfx_window_struct[1];
        if (newWindow == NULL)
            printf("Error\n");
            //Error out

        newWindow->pGLEWContext = NULL;
        newWindow->pWindow      = NULL;
        newWindow->uiID         = g_uiWindowCounter++;        //set ID and Increment Counter!
        newWindow->uiWidth      = disp_w;
        newWindow->uiHeight     = disp_h;
        newWindow->mode         = mode;

        // Initalize GLFW
        glfwSetErrorCallback(error_callback);
        if (!glfwInit()) {
            std::cerr << "ERROR: GLFW wasn't able to initalize" << std::endl;
            exit(EXIT_FAILURE);
        }

        // Add Hints
        glfwWindowHint(GLFW_DEPTH_BITS, mode * sizeof(T));
        glfwWindowHint(GLFW_RESIZABLE, false);

        // Create the window itself
        newWindow->pWindow = glfwCreateWindow(newWindow->uiWidth, newWindow->uiHeight, title, NULL, NULL);

        // Confirm window was created successfully:
        if (newWindow->pWindow == NULL)
        {
            printf("Error: Could not Create GLFW Window!\n");
            delete newWindow;
            return NULL;
        }

        // Create GLEW Context
        newWindow->pGLEWContext = new GLEWContext();
        if (newWindow->pGLEWContext == NULL)
        {
            printf("Error: Could not create GLEW Context!\n");
            delete newWindow;
            return NULL;
        }

        // Set context (before glewInit())
        MakeContextCurrent(newWindow);

        //GLEW Initialization - Must be done
        GLenum err = glewInit();
        if (err != GLEW_OK) {
            printf("GLEW Error occured, Description: %s\n", glewGetErrorString(err));
            glfwDestroyWindow(newWindow->pWindow);
            delete newWindow;
            return NULL;
        }

        int b_width  = newWindow->uiWidth;
        int b_height = newWindow->uiHeight;
        glfwGetFramebufferSize(newWindow->pWindow, &b_width, &b_height);

        glViewport(0, 0, b_width, b_height);

        glfwSetKeyCallback(newWindow->pWindow, key_callback);

        MakeContextCurrent(newWindow);

        CheckGL("At End of Create Window");
        return newWindow;
    }

#define INSTANTIATE(T)                                                                          \
    template afgfx_window createWindow<T>(const unsigned disp_h, const unsigned disp_w,         \
                                        const char *title, const afgfx_color_mode mode);

    INSTANTIATE(float);
    INSTANTIATE(int);
    INSTANTIATE(unsigned);
    INSTANTIATE(char);
    INSTANTIATE(unsigned char);

#undef INSTANTIATE

    void makeWindowCurrent(const afgfx_window window)
    {
        CheckGL("Before Make Window Current");
        MakeContextCurrent(window);
        CheckGL("In Make Window Current");
    }

    void destroyWindow(afgfx_window window)
    {
        CheckGL("Before Delete Window");
        // Cleanup
        MakeContextCurrent(window);

        // Delete GLEW context and GLFW window
        delete window->pGLEWContext;
        glfwDestroyWindow(window->pWindow);

        CheckGL("In Delete Window");
    }
}
