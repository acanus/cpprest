#include "NvInfer.h"
#include <iostream> 
#include "NvOnnxParser.h"
#include "NvUffParser.h"
using namespace nvonnxparser;
using namespace nvinfer1;
using namespace nvuffparser;
namespace deeplearning
{
    class Logger : public ILogger           
    {
        void log(Severity severity, const char* msg) override
        {
            // suppress info-level messages
            if (severity <= Severity::kWARNING)
                std::cout << msg << std::endl;
        }
    } logger;
    class segnet
    {
    private:
        vector<size_t> networkInputs;
        vector<size_t> networkOutputs;
        IUffParser* uffparser;
    public:
        void addInput(string layer, nvinfer1::DimsCHW dims,size_t eleSize) {
            /*
            Register tensorflow input
            Even if channel index is last in the data, put it first for TensorRT
            This network inputs are defined in Keras as follows:
            Input(shape=(Y, X, C))
            Where Y = 30, X = 40, C = 1
            */
            uffparser->registerInput(layer.c_str(), dims,nvuffparser::UffInputOrder::kNHWC);

            /* Save the size for inferences */
            //networkInputs.push_back(volume(dims) * eleSize);
        }
        void addOutput(string layer, size_t eleSize) {
            /*
            Name of last operation of last non optimizer layer found with
            `convert_to_uff.py tensorflow --input-file graph.pb -l`
            A dimension is not neccisary
            */
            uffparser->registerOutput(layer.c_str());

            /* Save the size for inferences */
            //networkOutputs.push_back(eleSize);
        }

        
        segnet(/* args */)
        {
            auto modelFile="";
            uffparser =  createUffParser();
            IBuilder* builder = createInferBuilder(logger);
            uint32_t flag = 1U <<static_cast<uint32_t>
            (NetworkDefinitionCreationFlag::kEXPLICIT_BATCH); 

            INetworkDefinition* network = builder->createNetworkV2(flag);
            IParser*  parser = createParser(*network, logger);
            parser->parseFromFile(modelFile, 2);
            for (int32_t i = 0; i < parser->getNbErrors(); ++i)
            {
            std::cout << parser->getError(i)->desc() << std::endl;
            }
            IBuilderConfig* config = builder->createBuilderConfig();
            config->setMaxWorkspaceSize(1U << 20);
            ICudaEngine*  serializedModel = builder->buildEngineWithConfig(*network, *config);
            
        }
        
        ~segnet()
        {
        }
    }; 
} // namespace deeplearning

