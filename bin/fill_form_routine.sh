#!/bin/bash

name=$1
fill=${name/_out.txt/}

if [ "`which wget`" == "" ]; then
	echo wget not available. This is required to download the forms.
	exit;
fi
if [ "`which md5sum`" == "" ]; then
	echo md5sum not available. This is required to check the forms.
	exit;
fi

if [ "`which pdftk`" == "" ]; then
	echo pdftk not available. This is required to fill out the forms.
	exit;
fi

fill_form()
{
form_xfdf=$1
form_pdf=$2
form_output=$3
md5=$4
error_msg=$5
url=$6

if [ -e $form_xfdf ]; then
	wget $url 
	if [ "`md5sum --tag $form_pdf`" != "$md5" ]; then
		echo $error_msg 
		md5sum --tag $form_pdf
		echo $md5
		exit;
	fi
	pdftk $form_pdf fill_form $form_xfdf output $form_output
fi
}

