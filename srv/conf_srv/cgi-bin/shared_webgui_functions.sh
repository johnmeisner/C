#!/bin/sh
#
## This is a shared library of k_print_*_tip functions to
#  be used between all the config cgi-bin scripts.

function k_print_text_input_tip {

    # Input parameters
    LABEL="$1"   #   $1 = LABEL
    ID="$2"      #   $2 = ID
    PHOLDER="$3" #   $3 = PHOLDER (placeholder/current value)
    TOOLTIP="$4" #   $4 = TOOLTIP

    echo "              <div class='form-group row' style='margin-bottom: 0px;' "
    echo "                data-toggle='tooltip' data-placement='top' title='$TOOLTIP'>"
    echo "                <label for='$ID' class='col-sm-4 col-form-label'>$LABEL </label>"
    echo "                <div class='col-sm-6'>"
    echo "                  <input type='text' class='form-control' name='$ID' id='$ID' placeholder='$PHOLDER' value='$PHOLDER' onchange=\"change_notify($ID)\" >"
    echo "                </div>"
    echo "              </div>"

}

function k_print_check_box_tip {

    # Input parameters
    LABEL="$1"   #   $1 = LABEL
    ID="$2"      #   $2 = ID
    VALUE="$3"   #   $3 = VALUE
    TOOLTIP="$4" #   $4 = TOOLTIP (TBD)

    if [ "$VALUE" == "0" ]; then 
        RESULT="" 
    else 
        RESULT="checked" 
    fi
    # adding change detect
    echo "            <div class=\"form-check\"><input class=\"form-check-input\" type=\"checkbox\" name=\"$ID\" id=\"$ID\" $RESULT onchange=\"change_notify($ID)\" >"
    echo "            <label class=\"form-check-label\" for=\"$ID\" "
    echo "            data-toggle=\"tooltip\" data-placement=\"top\" title=\"$TOOLTIP\"> "
    echo "            $LABEL</label></div>"
}

function k_print_block_header {

    # Input parameters
    NAME="$1"    #   $1 = NAME

    echo "   <div>"
    echo "            <label class=\"form-check-label\" for=\"$NAME\" "
    echo "            data-toggle=\"tooltip\" data-placement=\"top\" title=\"$NAME\"> "
    echo "            <b>$NAME</b></label>"
    echo "   </div>"
}

#============================================================================

