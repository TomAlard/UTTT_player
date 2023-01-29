import torch
import shutil

VERSION = 'v25M_NNUE_Quant'
MODEL_FILENAME = 'model_latest.pth'


def clamp(n, smallest, largest):
    return max(smallest, min(n, largest))


def double_to_int8(value):
    x = round(value * 64)
    if x > 127 or x < -128:
        print(value, x)
    return str(clamp(x, -128, 127))


def export2d(filename, parameters):
    with open(f'./parameters/{VERSION}/{filename}', 'w') as f:
        f.write('{')
        for i, line in enumerate(parameters):
            f.write('{')
            line = ','.join(map(double_to_int8, line))
            f.write(line + '}')
            if i+1 != len(parameters):
                f.write(',\n')
        f.write('}')


def export1d(filename, parameters):
    with open(f'./parameters/{VERSION}/{filename}', 'w') as f:
        line = ','.join(map(double_to_int8, parameters))
        f.write('{' + line + '}')


def main():
    model = torch.load(MODEL_FILENAME)
    print(model.l2.bias.shape)
    exit(0)
    export2d('hidden1_layer_weights.txt', model.l1.weight.T.tolist())
    export1d('hidden1_layer_biases.txt', model.l1.bias.tolist())
    export2d('hidden2_layer_weights.txt', model.l2.weight.T.tolist())
    export1d('hidden2_layer_biases.txt', model.l2.bias.tolist())
    export2d('hidden3_layer_weights.txt', model.l3.weight.T.tolist())
    export1d('hidden3_layer_biases.txt', model.l3.bias.tolist())
    export1d('output_layer_weights.txt', model.l4.weight.tolist()[0])
    with open(f'./parameters/{VERSION}/output_layer_bias.txt', 'w') as f:
        f.write(str(model.l4.bias.item()))
    # shutil.copyfile(MODEL_FILENAME, f'parameters/{VERSION}/{MODEL_FILENAME}')


if __name__ == '__main__':
    main()
