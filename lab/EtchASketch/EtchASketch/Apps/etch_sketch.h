/*
 * etch_sketch.h
 *
 * Created: 4/25/2026 12:31:40 PM
 *  Author: samou
 */ 


#ifndef ETCH_SKETCH_H_
#define ETCH_SKETCH_H_

#include <stdint.h>

/*
 * Etch A Sketch app lifecycle.
 *
 * Init is called once when the app is selected.
 * Update is called repeatedly from the main app runner.
 */
void EtchSketch_Init(void);
void EtchSketch_Update(void);

#endif /* ETCH_SKETCH_H_ */
