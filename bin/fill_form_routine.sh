#!/bin/bash

name=$1
fill=${name/_out.txt/}

verbose=0

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

# Modify this function to call md5sum on your distribution, and retain only the hash
do_md5sum()
{
  infile="$1"
  out=
  if [ -e /etc/redhat-release ]; then	# works on centos/rhel
	out=`md5sum $form_pdf | sed -e "s/ .*//"`
  else
      	out=`md5sum --tag "$infile" | sed -e "s/.*= //"`
  fi
  echo "$out"
}

fill_form()
{
form_xfdf=$1
form_pdf=$2
form_output=$3
md5=$4
error_msg=$5
url=$6

if [ "$verbose" = "1" ]; then
  echo "fill_form: called with :form_xfdf=$form_xfdf"
  echo "fill_form: called with :form_pdf=$form_pdf"
  echo "fill_form: called with :form_output=$form_output"
  echo "fill_form: called with :md5=$md5"
  echo "fill_form: called with :error_msg=$error_msg"
  echo "fill_form: called with :url=$url"
fi

if [ -e $form_xfdf ]; then
        if [ "$verbose" = "1" ]; then echo "found $form_xfdf"; fi
        if [ ! -e $form_pdf ]; then
           if [ "$verbose" = "1" ]; then echo "wget $url"; fi
	         wget $url 
	fi
	if [ "$verbose" = "1" ]; then echo "md5sum $form_pdf"; fi
	md5sumval=$(do_md5sum "$form_pdf")
	if [ "$md5sumval" != "$md5" ]; then
		echo $error_msg 
		echo "$md5sumval"
		echo $md5
		exit;
	fi
        if [ "$verbose" = "1" ]; then echo "pdftk $form_pdf fill_form $form_xfdf output $form_output"; fi
	pdftk $form_pdf fill_form $form_xfdf output $form_output
else
    echo "notfound $form_xfdf"
fi
}

