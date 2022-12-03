#include <QCoreApplication>
#include <QCommandLineParser>
#include <QImage>
#include <QtCore>
#include <QtConcurrent>

#include <iostream>
#include "utils/sceneparser.h"
#include "raytracer/raytracer.h"
#include "raytracer/raytracescene.h"


int main(int argc, char *argv[])
{
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    QCoreApplication a(argc, argv);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addPositionalArgument("config", "Path of the config file.");
    parser.process(a);

    auto positionalArgs = parser.positionalArguments();
    if (positionalArgs.size() != 1) {
        std::cerr << "Not enough arguments. Please provide a path to a config file (.ini) as a command-line argument." << std::endl;
        a.exit(1);
        return 1;
    }

    QSettings settings( positionalArgs[0], QSettings::IniFormat );
    QString iScenePath = settings.value("IO/scene").toString();
    QString oImagePath = settings.value("IO/output").toString();

    std::cout << "Parsing the scene" << std::endl;

    std::vector<RenderData*> metaData;
    bool success = SceneParser::parse(iScenePath.toStdString(), metaData);

    if (!success) {
        std::cerr << "Error loading scene: \"" << iScenePath.toStdString() << "\"" << std::endl;
        a.exit(1);
        return 1;
    }

    // Raytracing-relevant code starts here

    int width = settings.value("Canvas/width").toInt();
    int height = settings.value("Canvas/height").toInt();


    // Setting up the raytracer
    RayTracer::Config rtConfig{};
    rtConfig.enableShadow        = settings.value("Feature/shadows").toBool();
    rtConfig.enableReflection    = settings.value("Feature/reflect").toBool();
    rtConfig.enableRefraction    = settings.value("Feature/refract").toBool();
    rtConfig.enableTextureMap    = settings.value("Feature/texture").toBool();
    rtConfig.enableTextureFilter = settings.value("Feature/texture-filter").toBool();
    rtConfig.enableParallelism   = settings.value("Feature/parallel").toBool();
    rtConfig.enableSuperSample   = settings.value("Feature/super-sample").toBool();
    rtConfig.numSamples          = settings.value("Feature/num-samples").toInt();
    rtConfig.enablePostProcess   = settings.value("Feature/post-process").toBool();
    rtConfig.enableAcceleration  = settings.value("Feature/acceleration").toBool();
    rtConfig.enableDepthOfField  = settings.value("Feature/depthoffield").toBool();

    auto renderFrame = [&](int frame) {
        std::cout << "Rendering frame " << frame << std::endl;

        // Extracting data pointer from Qt's image API
        QImage image = QImage(width, height, QImage::Format_RGBX8888);
        image.fill(Qt::black);
        RGBA *data = reinterpret_cast<RGBA *>(image.bits());

        RayTracer raytracer{ rtConfig };
        RayTraceScene rtScene{ width, height, *metaData[frame] };

        // Note that we're passing `data` as a pointer (to its first element)
        // Recall from Lab 1 that you can access its elements like this: `data[i]`
        raytracer.render(data, rtScene);

        QString number = QStringLiteral("%1").arg(frame, 5, 10, QLatin1Char('0'));
        std::cout << (oImagePath + number + ".png").toStdString() <<  std::endl;

        bool success = image.save(oImagePath + number + ".png", "PNG");

        if (success) {
            std::cout << "Saved rendered image to \"" << oImagePath.toStdString() << "\"" << std::endl;
        } else {
            std::cerr << "Error: failed to save image to \"" << oImagePath.toStdString() << "\"" << std::endl;
        }

        return;
    };



//    if (rtConfig.enableParallelism) {
//        std::vector<int> frameNums;
//        for (int i = 0; i < ceil(metaData[0]->globalData.duration * metaData[0]->globalData.framerate); i++) {
//            frameNums.push_back(i);
//        }

//        QtConcurrent::blockingMap(frameNums, renderFrame);
//    } else {
        // otherwise just use on single thread.
        for (int i = 0; i < ceil(metaData[0]->globalData.duration * metaData[0]->globalData.framerate); i++) {
            renderFrame(i);
        }
//    }

    a.exit();
    return 0;
}
