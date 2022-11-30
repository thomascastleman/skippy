
# Overview

We will produce .GIF files by rendering each frame individually with our raytracers, and using information specified in the scenefiles to interpolate between positions and scales (and possibly maybe rotation). 

In the code, this will correspond to changing the scenefile format and its parser to accommodate specification of scaling/translation values at specific frame "times." Then, we will need to compute (based on the chosen framerate and GIF duration) as many scenes (with different positions / scales / etc for the primitives) as there are frames in the output. Then, we will render each of these frames in parallel (using our normal raytracer). Finally, we will synthesize all of the rendered frames into a .GIF file, which will be the output of our program.

# Division of Labor / Plan of Action

- [ ] Modifying the parser
  - [ ] Framerate (global)
  - [ ] Duration (global)
  - [ ] "Keyframe" data that indicates transformations at key frames

- [ ] Performing the interpolation
  - [ ] Scale interpolation
  - [ ] Translation interpolation
  - [ ] Rotation interpolation? (v2)
  - [ ] Non-linear interpolation? (v2)

- [ ] Use FFMPEG to produce video from separate images
  - https://stackoverflow.com/questions/24961127/how-to-create-a-video-from-images-with-ffmpeg

As the first two parts listed above are most of the project's complexity and are pretty intertwined, we plan to pair program most of the project.

### Where will we start?

Likely changing the parser to accommodate our new format. Then, interpolation. Then, output format.

### Implementation Ideas

- Augment the internal representation of scenes so that each `SceneTransformation` actually contains functions that perform interpolation between values for a given kind of transformation (for instance, a function that interpolates between different translation values at t = 0, t = 5, and t = 10). Given a time value, these functions would compute what the transformation value would be at that frame. We will use these to generate each of the frame's scenefile representations.

- As soon as the total number of frames has been determined, pretty much all of the computation can proceed in parallel, which we hope to take advantage of. Only at the end, when all the rendered frames have been produced, do we need to combined them all into a single GIF serially.

- For creating a GIF, we will need to read up on the file format and also look at libraries we could use for generating them (ideally). We will also need to determine how to scale our color down into 8-bit color.


### Disaster Plan

If we are unable to synthesize GIFs for whatever reason, then we will just produced as many images as there are frames, and use some external tool to combine them into GIF files.

