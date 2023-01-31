import torch
import shutil

VERSION = 'v25M_256_4096'
MODEL_FILENAME = 'model_latest.pth'


def clamp(n, smallest, largest):
    return max(smallest, min(n, largest))


def double_to_int(value, scaling_factor, smallest, largest):
    x = round(value * scaling_factor)
    if (x > largest or x < smallest) and scaling_factor == 64:
        print(value, x)
    return str(clamp(x, smallest, largest))


def export2d(filename, parameters, scaling_factor, smallest, largest):
    with open(f'./parameters/{VERSION}/{filename}', 'w') as f:
        f.write('{')
        for i, line in enumerate(parameters):
            f.write('{')
            line = ','.join((double_to_int(v, scaling_factor, smallest, largest) for v in line))
            f.write(line + '}')
            if i+1 != len(parameters):
                f.write(',\n')
        f.write('}')


def export1d(filename, parameters, scaling_factor, smallest, largest):
    with open(f'./parameters/{VERSION}/{filename}', 'w') as f:
        line = ','.join((double_to_int(v, scaling_factor, smallest, largest) for v in parameters))
        f.write('{' + line + '}')


def main():
    model = torch.load(MODEL_FILENAME)
    export2d('hidden_layer_weights.txt', model.l1.weight.T.tolist(), 127, -32768, 32767)
    export1d('hidden_layer_biases.txt', model.l1.bias.tolist(), 127, -32768, 32767)
    export1d('output_layer_weights.txt', model.l2.weight.tolist()[0], 64, -127, 128)
    with open(f'./parameters/{VERSION}/output_layer_bias.txt', 'w') as f:
        f.write(str(model.l2.bias.item()))
    shutil.copyfile(MODEL_FILENAME, f'parameters/{VERSION}/{MODEL_FILENAME}')


if __name__ == '__main__':
    main()
