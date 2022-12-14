# skippy

It's Skippy, not Jif.

Final project for CSCI 1230, created by Stewart Morris and Thomas Castleman, fall 2022.

## Scenes

Our animated scenefiles can be found in the `scenes/` subdirectory.

## Output

Rendered frames and output video files can be found in the `output/` subdirectory.

## Usage

First, update `QSettings.ini` to ensure that it has the path of the scenefile you want to render. The 
`output` field should be the name of an output directory where frames and an `.mp4` file will be deposited.

Then, (assuming the raytracer has already been built in QT Creator) you can use our script to invoke both the raytracer
and `ffmpeg` using the following command:

```bash
./render_video
```

## Third Party Libraries

For synthesizing video from our still frames, we used the [`ffmpeg`](https://ffmpeg.org/) tool.

## Demo

A demo video describing the project can be found [here](https://drive.google.com/file/d/1dErHTrLQQAgvm4InHaO2_YroEzMm1WBM/view?usp=sharing).