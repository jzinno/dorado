#!/bin/bash

# Test expected log output from the dorado binary execution.

set -ex
set -o pipefail

test_dir=$(dirname $0)
dorado_bin=$(cd "$(dirname $1)"; pwd -P)/$(basename $1)
model=${2:-dna_r9.4.1_e8_hac@v3.3}
batch=${3:-384}
data_dir=$test_dir/data
output_dir=${test_dir}/test_output
mkdir -p $output_dir

test_output_file=$test_dir/output.log

echo dorado basecaller test stage
$dorado_bin download --model ${model}
$dorado_bin basecaller ${model} $data_dir/pod5 -b ${batch} --emit-fastq > $output_dir/ref.fq
$dorado_bin basecaller ${model} $data_dir/pod5 -b ${batch} --modified-bases 5mCG --emit-moves > $output_dir/calls.bam
if ! uname -r | grep -q tegra; then
    $dorado_bin basecaller ${model} $data_dir/pod5 -x cpu --modified-bases 5mCG > $output_dir/calls.bam
fi
samtools quickcheck -u $output_dir/calls.bam
samtools view $output_dir/calls.bam > $output_dir/calls.sam

echo dorado aligner test stage
$dorado_bin aligner $output_dir/ref.fq $output_dir/calls.sam > $output_dir/calls.bam
$dorado_bin basecaller ${model} $data_dir/pod5 -b ${batch} --modified-bases 5mCG | $dorado_bin aligner $output_dir/ref.fq > $output_dir/calls.bam
$dorado_bin basecaller ${model} $data_dir/pod5 -b ${batch} --modified-bases 5mCG --reference $output_dir/ref.fq > $output_dir/calls.bam
samtools quickcheck -u $output_dir/calls.bam
samtools view -h $output_dir/calls.bam > $output_dir/calls.sam

rm -rf $output_dir
