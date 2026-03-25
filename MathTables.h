
/*

This file contains pre-generated lookup tables.

To regenerate the values, paste these JavaScript routines in the JavaScript
console of a web browser:

  function spacedSequence(min, max, count) {
    const step = (max - min) / (count - 1);
    return Array.from({ length: count }, (_, i) => min + i * step);
  }

  function makeSequence(func, min, max, count, range) {
    return spacedSequence(min, max, count).map(func).map(x => Math.round(x * range))
  }

  function bounce(seq) {
    return seq.concat(seq.slice().reverse());
  }

  function easeInQuad(x)     {return x * x;}
  function easeOutQuad(x)    {return 1 - (1 - x) * (1 - x);}
  function easeOutQuint(x)   {return 1 - Math.pow(1 - x, 5);}
  function easeInOutCubic(x) {return x < 0.5 ? 4 * x * x * x : 1 - Math.pow(-2 * x + 2, 3) / 2;}
  function easeInExpo(x)     {return x === 0 ? 0 : Math.pow(2, 10 * x - 10);}
  function easeOutExpo(x )   {return x === 1 ? 1 : 1 - Math.pow(2, -10 * x);}

  Reference: https://easings.net/
*/

#include <assert.h>

#define easeFuncBits 8
#define trigFuncBits 7

// console.log (makeSequence (easeInQuad, 0, 1, 64, 255 ));

const unsigned char easeInQuad[] = {
    0,    0,    0,    1,    1,   2,    2,    3,
    4,    5,    6,    8,    9,  11,   13,   14,
   16,   19,   21,   23,   26,  28,   31,   34,
   37,   40,   43,   47,   50,  54,   58,   62,
   66,   70,   74,   79,   83,  88,   93,   98,
  103,  108,  113,  119,  124, 130,  136,  142,
  148,  154,  161,  167,  174, 180,  187,  194,
  201,  209,  216,  224,  231, 239,  247,  255
};

// console.log (makeSequence (easeOutQuad, 0, 1, 64, 255 ));

const unsigned char easeOutQuad[] = {
    0,    8,   16,   24,   31,   39,   46,   54,
   61,   68,   75,   81,   88,   94,  101,  107,
  113,  119,  125,  131,  136,  142,  147,  152,
  157,  162,  167,  172,  176,  181,  185,  189,
  193,  197,  201,  205,  208,  212,  215,  218,
  221,  224,  227,  229,  232,  234,  236,  239,
  241,  242,  244,  246,  247,  249,  250,  251,
  252,  253,  253,  254,  254,  255,  255,  255
};

#if 0
// console.log (makeSequence (easeInOutCubic, 0, 1, 64, 255 ));

const unsigned char easeInOutCubic[] = {
    0,    0,    0,    0,    0,    1,    1,    1,
    2,    3,    4,    5,    7,    9,   11,   14,
   17,   20,   24,   28,   33,   38,   43,   50,
   56,   64,   72,   80,   90,   99,  110,  122,
  133,  145,  156,  165,  175,  183,  191,  199,
  205,  212,  217,  222,  227,  231,  235,  238,
  241,  244,  246,  248,  250,  251,  252,  253,
  254,  254,  254,  255,  255,  255,  255,  255
};
#endif

// console.log ( bounce( makeSequence (easeOutQuint, 0, 1, 32, 255 )));

const unsigned char easeBounceQuint[] = {
    0,   39,   72,  102,  127,  149,  168,  184,
  198,  209,  219,  226,  233,  238,  242,  246,
  248,  250,  252,  253,  254,  254,  254,  255,
  255,  255,  255,  255,  255,  255,  255,  255,
  255,  255,  255,  255,  255,  255,  255,  255,
  255,  254,  254,  254,  253,  252,  250,  248,
  246,  242,  238,  233,  226,  219,  209,  198,
  184,  168,  149,  127,  102,   72,   39,    0
};

#if 0
// console.log (makeSequence (easeInExpo, 0, 1, 64, 255 ));

const unsigned char easeInExpo[] = {
    0,    0,    0,    0,    0,    0,    0,    1,
    1,    1,    1,    1,    1,    1,    1,    1,
    1,    2,    2,    2,    2,    3,    3,    3,
    3,    4,    4,    5,    5,    6,    7,    8,
    8,    9,   10,   12,   13,   15,   16,   18,
   20,   23,   25,   28,   32,   35,   39,   44,
   49,   55,   61,   68,   76,   85,   95,  106,
  118,  132,  147,  164,  183,  205,  228,  255
};

// console.log (makeSequence (easeOutExpo, 0, 1, 64, 255 ));

const unsigned char easeOutExpo[] = {
    0,   27,   50,   72,   91,  108,  123,   137,
  149,  160,  170,  179,  187,  194,  200,   206,
  211,  216,  220,  223,  227,  230,  232,   235,
  237,  239,  240,  242,  243,  245,  246,   247,
  247,  248,  249,  250,  250,  251,  251,   252,
  252,  252,  252,  253,  253,  253,  253,   254,
  254,  254,  254,  254,  254,  254,  254,   254,
  254,  255,  255,  255,  255,  255,  255,   255
};
#endif

// console.log (makeSequence (Math.sin, 0, 2 * Math.PI, 64, 127 ) );

const signed char sineTable[] = {
    0,   13,   25,   37,   49,   61,   72,    82,
   91,   99,  107,  113,  118,  122,  125,   127,
  127,  126,  124,  120,  116,  110,  103,    95,
   86,   77,   66,   55,   43,   31,   19,     6,
   -6,  -19,  -31,  -43,  -55,  -66,  -77,   -86,
  -95, -103, -110, -116, -120, -124, -126,  -127,
 -127, -125, -122, -118, -113, -107,  -99,   -91,
  -82,  -72,  -61,  -49,  -37,  -25,  -13,    -0
};
