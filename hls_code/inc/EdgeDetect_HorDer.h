/**************************************************************************
 *                                                                        *
 *  Edge Detect Design Walkthrough for HLS                                *
 *                                                                        *
 *  Software Version: 1.0                                                 *
 *                                                                        *
 *  Release Date    : Tue Jan 14 15:40:43 PST 2020                        *
 *  Release Type    : Production Release                                  *
 *  Release Build   : 1.0.0                                               *
 *                                                                        *
 *  Copyright 2020, Siemens                                               *
 *                                                                        *
 **************************************************************************
 *  Licensed under the Apache License, Version 2.0 (the "License");       *
 *  you may not use this file except in compliance with the License.      *
 *  You may obtain a copy of the License at                               *
 *                                                                        *
 *      http://www.apache.org/licenses/LICENSE-2.0                        *
 *                                                                        *
 *  Unless required by applicable law or agreed to in writing, software   *
 *  distributed under the License is distributed on an "AS IS" BASIS,     *
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or       *
 *  implied.                                                              *
 *  See the License for the specific language governing permissions and   *
 *  limitations under the License.                                        *
 **************************************************************************
 *                                                                        *
 *  The most recent version of this package is available at github.       *
 *                                                                        *
 *************************************************************************/
#pragma once

#include "EdgeDetect_defs.h"
#include <mc_scverify.h>

namespace EdgeDetect_IP 
{
  class EdgeDetect_HorDer
  {
  public:
    EdgeDetect_HorDer() {}
  
    #pragma hls_design interface
    void CCS_BLOCK(run)(ac_channel<pixelType4x> &dat_in,
                        maxWType                &widthIn,
                        maxHType                &heightIn,
                        ac_channel<pixelType4x> &dat_out,
                        ac_channel<gradType4x>  &dx)
    {
      pixelType4x pix_in;
      gradType4x  grad4;
      // pixel buffers store pixel history
      pixelType pix_buf0;
      pixelType pix_buf1;
  
      pixelType pix0 = 0;
      pixelType pix1 = 0;
      pixelType pix2 = 0;
  
      gradType  pix[4];
  
      HROW: for (maxHType y = 0; ; y++) {
        #pragma hls_pipeline_init_interval 1
        HCOL: for (maxWType x = 0; ; x++) { // HCOL has one extra iteration to ramp-up window
          if (x <= widthIn/4-1) {
            pix_in = dat_in.read(); // Read streaming interface
            dat_out.write(pix_in);
          }
          
          for(uint2 i=0;;i++){
            pix0 = pix_in.slc<8>(i*8);
            pix2 = pix_buf1;
            pix1 = pix_buf0;
            if (x == 1) {
              pix2 = pix1; // left boundary (replicate pix1 left to pix2)
            }
            if (x == widthIn) {
              pix0 = pix1; // right boundary (replicate pix1 right to pix0)
            }
    
            pix_buf1 = pix_buf0;
            pix_buf0 = pix0;
    
            // Calculate derivative
            pix[i] = pix2*kernel[0] + pix1*kernel[1] + pix0*kernel[2];
    
            if(i==3)break;
          }
          if (x != 0) { // Write streaming interface
            grad4.grad0 = pix[0];
            grad4.grad1 = pix[1];
            grad4.grad2 = pix[2];
            grad4.grad3 = pix[3];
            dx.write(grad4); // derivative out
          }
          // programmable width exit condition
          if ( x == widthIn/4) {
            break;
          }
        }
        // programmable height exit condition
        if (y == maxHType(heightIn-1)) { // cast to maxHType for RTL code coverage
          break;
        }
      }
    }
  };

}


