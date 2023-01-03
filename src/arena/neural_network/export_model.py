import torch
import shutil

VERSION = 'v25M_256'
MODEL_FILENAME = 'parameters/v25M_256/model_latest.pth'


def export2d(filename, parameters):
    with open(f'./parameters/{VERSION}/{filename}', 'w') as f:
        f.write('{')
        for i, line in enumerate(parameters):
            f.write('{')
            line = ','.join(f'{x}f' for x in line)
            f.write(line + '}')
            if i+1 != len(parameters):
                f.write(',\n')
        f.write('}')


def export1d(filename, parameters):
    with open(f'./parameters/{VERSION}/{filename}', 'w') as f:
        line = ','.join(f'{x}f' for x in parameters)
        f.write('{' + line + '}')


def main():
    model = torch.load(MODEL_FILENAME)
    layers = model.linear_relu_stack
    export2d('hidden_layer_weights.txt', layers[0].weight.T.tolist())
    export1d('hidden_layer_biases.txt', layers[0].bias.tolist())
    export1d('output_layer_weights.txt', layers[2].weight.tolist()[0])
    export1d('output_layer_biases.txt', layers[2].bias.tolist())
    # shutil.copyfile(MODEL_FILENAME, f'parameters/{VERSION}/{MODEL_FILENAME}')


if __name__ == '__main__':
    main()
