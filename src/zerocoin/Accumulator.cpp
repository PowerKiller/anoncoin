/**
 * @file       Accumulator.cpp
 *
 * @brief      Accumulator and AccumulatorWitness classes for the Zerocoin library.
 *
 * @author     Ian Miers, Christina Garman and Matthew Green
 * @date       June 2013
 *
 * @copyright  Copyright 2013 Ian Miers, Christina Garman and Matthew Green
 * @license    This project is released under the MIT license.
 **/

#include <iostream>     // GNOSIS DEBUG
#include <sstream>
#include <sys/time.h>
#include "../Zerocoin.h"

namespace libzerocoin {

//Accumulator class
Accumulator::Accumulator(const AccumulatorAndProofParams* p, const CoinDenomination d): params(p), denomination(d) {
	if (!(params->initialized)) {
		throw ZerocoinException("Invalid parameters for accumulator");
	}

	this->value = this->params->accumulatorBase;
}

Accumulator::Accumulator(const Params* p, const CoinDenomination d) {
	this->params = &(p->accumulatorParams);
	this->denomination = d;

	if (!(params->initialized)) {
		throw ZerocoinException("Invalid parameters for accumulator");
	}

	this->value = this->params->accumulatorBase;
}

void Accumulator::accumulate(const PublicCoin& coin) {
	// Make sure we're initialized
	if(!(this->value)) {
		throw ZerocoinException("Accumulator is not initialized");
	}

	if(this->denomination != coin.getDenomination()) {
		//std::stringstream msg;
		std::string msg;
		msg = "Wrong denomination for coin. Expected coins of denomination: ";
		msg += this->denomination.getValue();
		msg += ". Instead, got a coin of denomination: ";
		msg += coin.getDenomination().getValue();
		throw ZerocoinException(msg);
	}

	if(coin.validate()) {
		struct timeval tv0, tv1;
		double elapsed;
		// Compute new accumulator = "old accumulator"^{element} mod N
		gettimeofday(&tv0, NULL);
		this->value = this->value.pow_mod(coin.getValue(), this->params->accumulatorModulus);
		gettimeofday(&tv1, NULL);
		elapsed = (tv1.tv_sec  - tv0.tv_sec) +
		          (tv1.tv_usec - tv0.tv_usec) / 1e6;
		std::cout << "GNOSIS DEBUG: accumulate time: " << elapsed << std::endl;

	} else {
		throw ZerocoinException("Coin is not valid");
	}
}

CoinDenomination Accumulator::getDenomination() const {
	return this->denomination;
}

Bignum Accumulator::getValue() const {
	return this->value;
}

Accumulator& Accumulator::operator += (const PublicCoin& c) {
	this->accumulate(c);
	return *this;
}

bool Accumulator::operator == (const Accumulator rhs) const {
	return this->value == rhs.value;
}

//AccumulatorWitness class
AccumulatorWitness::AccumulatorWitness(const Params* p,
                                       const Accumulator& checkpoint, const PublicCoin coin): params(p), witness(checkpoint), element(coin) {
}

void AccumulatorWitness::addElement(const PublicCoin& c) {
	if(element != c) {
		witness += c;
	}
}

Bignum AccumulatorWitness::getValue() const {
	return this->witness.getValue();
}

bool AccumulatorWitness::verifyWitness(const Accumulator& a, const PublicCoin &publicCoin) const {
	Accumulator temp(witness);
	temp += element;
	return (temp == a && this->element == publicCoin);
}

AccumulatorWitness& AccumulatorWitness::operator +=(
    const PublicCoin& rhs) {
	this->addElement(rhs);
	return *this;
}

} /* namespace libzerocoin */