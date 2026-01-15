#include <SDL2/SDL.h>

// Create an error window with a custom message
#define error_box(message) \
  SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,			\
			   "Fatal Error",				\
			   message,					\
			   nullptr);					

// Asserts if expression is true
// Creates an error window with a custom message, and then exits the program.
#define error_assert(expression, message)				\
  if (expression) {							\
    error_box(message);							\
    exit(1);								\
  }
