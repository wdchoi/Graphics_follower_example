/////////////////////////////////////////////////////////////////////////////
// Authored by Jeong-Mo Hong for CSE4060 course at Dongguk University CSE  //
// jeongmo.hong@gmail.com                                                  //
// Do whatever you want license.                                           //
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <iostream>

#include "Array1D.h"
#include "MatrixMN.h"
//#include "Utilities\RandomNumberGenerator.h"
//#include "DataStructure\CircularQueue.h"

#define MAX2(a, b) ((a) > (b) ? (a) : (b))

typedef double D;

class NeuralNetwork
{
public:
    int num_input_;
    int num_output_;
    int num_all_layers_; // num_all_layers_ = num_hidden_layers_ + 2

    D   bias_;  // constant bias

    D   eta_;   // learning rate
    D   alpha_; // momentum term coefficient

    Array1D<VectorND<D> > layer_neuron_act_;         // layer_neuron_act_[0] = input layer, layer_neuron_act_[num_all_layers_-1] = output_layer, layer_neuron_act_[ix_layer][ix_neuron] = activation value
    Array1D<VectorND<D> > layer_neuron_grad_;        // gradient values for back propagation
    Array1D<MatrixMN<D> > weights_;                  // weights_[0] is between layer 0 and layer 1. 

    VectorND<unsigned>    num_layer_acts_;           // The number of activation values of each layer. This includes bias.
   
    NeuralNetwork()
    {}

    NeuralNetwork(const int& _num_input, const int& _num_output, const int& _num_hidden_layers)
    {
        initialize(_num_input, _num_output, _num_hidden_layers);
    }

    void initialize(const int& _num_input, const int& _num_output, const int& _num_hidden_layers)
    {
        num_layer_acts_.initialize(_num_hidden_layers + 2);

        num_layer_acts_[0] = _num_input + 1;                        // layer 0 is input layer, +1 is for bias
        for (int l = 1; l < _num_hidden_layers + 1; ++l)
        {
            num_layer_acts_[l] = _num_input + 1;                    // default value
        }

        num_layer_acts_[_num_hidden_layers + 1] = _num_output + 1;   // last layer is output layer. Add +1 for bias as well in case this NN is combined with others.

        initialize(num_layer_acts_, _num_hidden_layers);
    }

    void initialize(const VectorND<unsigned>& num_layer_acts_, const int& _num_hidden_layers)
    {
        num_input_ = num_layer_acts_[0] - 1;                         // -1 is for bias
        num_output_ = num_layer_acts_[_num_hidden_layers + 1] - 1;   // -1 is for bias
        num_all_layers_ = _num_hidden_layers + 2; // hidden layers + 1 input layer + 1 output layer

        bias_  = 1;
        eta_   = 0.15;
        alpha_ = 0.5;

        // initialize all layers
        layer_neuron_act_.initialize(num_all_layers_);                           
        for (int l = 0; l < num_all_layers_; ++l)
        {
            layer_neuron_act_[l].initialize(num_layer_acts_[l], true);
            layer_neuron_act_[l][num_layer_acts_[l] - 1] = 1.0;     // bias
        }

        // initialize to store gradient of layers
        layer_neuron_grad_.initialize(num_all_layers_);
        for (int l = 0; l < num_all_layers_; ++l)
            layer_neuron_grad_[l].initialize(num_layer_acts_[l], true);

        // initialize weight matrix between layers
        weights_.initialize(num_all_layers_ - 1);                  // Note -1. Weight matrices are between layers.
        for (int l = 0; l < weights_.num_elements_; l++)
        {
            // row x column = (dimension of next layer  - 1 for bias) x  (dimension of prev layer - this includes bias)
            weights_[l].initialize(layer_neuron_act_[l + 1].num_dimension_ - 1, layer_neuron_act_[l].num_dimension_);// -1 is for bias. y = W [x b]^T. Don't subtract 1 if you want [y b]^T = W [x b]^T.

            // random initialization
            for (int ix = 0; ix < weights_[l].num_rows_ * weights_[l].num_cols_; ix++)
                weights_[l].values_[ix] = (double)rand() / RAND_MAX * 0.1;
        }
    }

    D getSigmoid(const D& x)
    {
        return 1.0 / (1.0 + exp(-x));
    }

    D getSigmoidGradFromY(const D& y)   // not from x. y = getSigmoid(x).
    {
        return (1.0 - y) * y;
    }

    D getRELU(const D& x)
    {
        return MAX2(0.0, x);
    }

    D getRELUGradFromY(const D& x) // RELU Grad from X == RELU Grad from Y
    {
        if (x > 0.0) return 1.0;
        else return 0.0;
    }

    D getLRELU(const D& x)
    {
        return x > 0.0 ? x : 0.01*x;
    }

    D getLRELUGradFromY(const D& x) // RELU Grad from X == RELU Grad from Y
    {
        if (x > 0.0) return 1.0;
        else return 0.01;
    }

    void applySigmoidToVector(VectorND<D>& vector)
    {
        for (int d = 0; d < vector.num_dimension_ - 1; d++) // don't apply activation function to bias
            vector[d] = getSigmoid(vector[d]);
    }

    void applyRELUToVector(VectorND<D>& vector)
    {
        for (int d = 0; d < vector.num_dimension_ - 1; d++) // don't apply activation function to bias
            vector[d] = getRELU(vector[d]);
    }

    void applyLRELUToVector(VectorND<D>& vector)
    {
        for (int d = 0; d < vector.num_dimension_ - 1; d++) // don't apply activation function to bias
            vector[d] = getLRELU(vector[d]);
    }

    void feedForward()
    {
        for (int l = 0; l < weights_.num_elements_; l++)
        {
            // The last component of layer_neuron_act_[l + 1], bias, shouldn't be updated. 
            weights_[l].multiply(layer_neuron_act_[l], layer_neuron_act_[l + 1]);

            //applySigmoidToVector(layer_neuron_act_[l + 1]);
			//applyLRELUToVector(layer_neuron_act_[l + 1]);
            applyRELUToVector(layer_neuron_act_[l + 1]);
        }
    }

	// backward propagation
	void propBackward(const VectorND<D>& target)
	{
		// calculate gradients of output layer
		{const int l = layer_neuron_grad_.num_elements_ - 1;
		for (int d = 0; d < layer_neuron_grad_[l].num_dimension_ - 1; d++)  // skip last component (bias)
		{
			const D &output_value(layer_neuron_act_[l][d]);
			layer_neuron_grad_[l][d] = (target[d] - output_value) * getRELUGradFromY(output_value);
			//layer_neuron_grad_[l][d] = (target[d] - output_value) * getLRELUGradFromY(output_value);
			//layer_neuron_grad_[l][d] = (target[d] - output_value) * getSigmoidGradFromY(output_value);
		}}

		// calculate gradients of hidden layers
		for (int l = weights_.num_elements_ - 1; l >= 0; l--)
		{
			weights_[l].multiplyTransposed(layer_neuron_grad_[l + 1], layer_neuron_grad_[l]);   //TODO: Transposed was missing before. debug!

			for (int d = 0; d < layer_neuron_act_[l].num_dimension_ - 1; d++)   // skip last component (bias)
			{
				layer_neuron_grad_[l][d] *= getRELUGradFromY(layer_neuron_act_[l][d]);
				//layer_neuron_grad_[l][d] *= getSigmoidGradFromY(layer_neuron_act_[l][d]);
				//ayer_neuron_grad_[l][d] *= getLRELUGradFromY(layer_neuron_act_[l][d]);
			}
		}

		// update weights
		for (int l = weights_.num_elements_ - 1; l >= 0; l--)
		{
			// correct weight values of matrix from layer l + 1 to l
			updateWeight(weights_[l], layer_neuron_grad_[l + 1], layer_neuron_act_[l]);
		}
	}

    void updateWeight(MatrixMN<D>& weight_matrix, VectorND<D>& next_layer_grad, VectorND<D>& prev_layer_act)
    {
        for (int row = 0; row < weight_matrix.num_rows_; row++)
        {
            for (int col = 0; col < weight_matrix.num_cols_; col++)
            {
                const D delta_w = eta_ * next_layer_grad[row] * prev_layer_act[col];

                weight_matrix.getValue(row, col) += delta_w;
            }
        }
    }

    void setInputVector(const VectorND<D>& input)
    {
        // use num_input_ in case input vector doesn't include bias

        if (input.num_dimension_ < num_input_)
            std::cout << "Input dimension is wrong" << std::endl;

        for (int d = 0; d < num_input_; d ++)
            layer_neuron_act_[0][d] = input[d];
    }

    void copyOutputVector(VectorND<D>& copy, bool copy_bias = false)
    {
        const VectorND<D>& output_layer_act(layer_neuron_act_[layer_neuron_act_.num_elements_ - 1]);

        if (copy_bias == false)
        {
            copy.initialize(num_output_, false);

            for (int d = 0; d < num_output_; d++)
                copy[d] = output_layer_act[d];
        }
        else
        {
            copy.initialize(num_output_ + 1, false);

            for (int d = 0; d < num_output_ + 1; d++)
                copy[d] = output_layer_act[d];
        }
    }

	int getIXProbabilistic(const VectorND<D>& input_vector)
	{
		VectorND<D> possibility;
		possibility.initialize(input_vector.num_dimension_, true);

		D sum = 0;

		for (int d = 0; d < input_vector.num_dimension_; d++)
		{
			sum += input_vector[d];
		}

		if (sum == 0.0) return 0;

		D accum = 0.0;

		for (int d = 0; d < input_vector.num_dimension_; d++)
		{
			accum += input_vector[d] / sum;

			possibility[d] = accum;
		}

		const D r = (D)rand() / RAND_MAX;

		for (int d = 0; d < input_vector.num_dimension_; d++)
		{
			if (r < possibility[d]) return d;
		}

		return input_vector.num_dimension_ - 1;
	}
};