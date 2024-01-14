#
# Use this script to inject a navigation bar to the web pages
# Injects a Navbar with javascript for each link.  There are two ways
#    to disable the navbar, so user cannot nav out of unsaved changes, and
#    so user cannot nav out with RSU needing to be changed back from a 
#    temporary usuable state.
#
# INSTRUCTIONS:
#    This script should be sourced, from inside other cgi-bin scripts, once the
#    <body> has been output, before anything else is drawn
#    In the "onload" javascript function for your page, as necessary,
#       call the "DEVELOPER CALLABLE FUNCTION" functions below, to customize
#       navbar behavior.
#
# NOTE: Contents of this script has been tested on Chrome, Edge, and Firefox
# black
# <nav class="navbar navbar-expand-sm bg-dark navbar-dark fixed-top" style="height:40px">
# red banner
#<nav class="navbar navbar-expand-sm bg-danger navbar-danger fixed-top" style="height:40px">
# light banner
#<nav class="navbar navbar-expand-sm bg-light navbar-light fixed-top" style="height:40px">
cat << '##EOFEOFEOF1'
<nav class="navbar navbar-expand-sm bg-light navbar-light fixed-top" style="height:40px">
    <a class="navbar-brand" href="/cgi-bin/rsu_main" onclick="navitem_clicked(this);">
       <img src="/assets/img/MobiQ_326x64.png" alt="Logo" style="height:32px;">
    </a>

    <!-- Links -->
    <button class="navbar-toggler" type="button" data-toggle="collapse" data-target="#navbarNav">
        <span class="navbar-toggler-icon"></span>
    </button>
    <div class="collapse navbar-collapse" id="navbarNav">
        <ul class="navbar-nav mr-auto">
            <li class="nav-item">
              <a class="nav-link" href="/cgi-bin/config_display_default" onclick="navitem_clicked(this);">Config</a>
            </li>
            <li class="nav-item">
              <a class="nav-link" href="/cgi-bin/config_advanced_live" onclick="navitem_clicked(this);">AdvConfig</a>
            </li>
            <li class="nav-item">
              <a class="nav-link" href="/cgi-bin/status_display_live" onclick="navitem_clicked(this);">Status</a>
            </li>
            <li class="nav-item">
              <a class="nav-link" href="/cgi-bin/status_advanced_live" onclick="navitem_clicked(this);">AdvStatus</a>
            </li>
            <li class="nav-item">
              <a class="nav-link" href="/cgi-bin/util_display_live" onclick="navitem_clicked(this);">Utilities</a>
            </li>
        </ul>
        <ul class="navbar-nav">
            <li class="nav-item">
                <a class="nav-link" href="/cgi-bin/user_logout" onclick="navitem_clicked(this);">Logout</a>
            </li>
        </ul>
    </div>
</nav>
<script>
var navbar__page_has_unsaved_changes = false;
var navbar__unsaved_changes_message = "You have unsaved changes. Click OK to continue (and lose changes), or Cancel to remain on this page (with unsaved changes)";
var navbar__the_navbar_is_disabled = false;
var navbar__disabled_navbar_message = "The navbar is disabled on this page";
function navitem_clicked(navitem)
{
    if (navbar__the_navbar_is_disabled) {
        // Block the href from going through
        event.preventDefault();
        // Tell user navbar is down
        alert(navbar__disabled_navbar_message);
    }
    if (navbar__page_has_unsaved_changes) {
        // Ask user if they really want to leave
        var ret = confirm(navbar__unsaved_changes_message);
        // If user hit cancel, cancel the href click event so user stays on page
        if (ret == false) {
            event.preventDefault();
        }
    }
}
// DEVELOPER CALLABLE FUNCTION - Tells navbar if there are
//    unsaved changes on the page.
function navbar_set_page_has_unsaved_changes(bool_value)
{
    navbar__page_has_unsaved_changes = bool_value;
}
// DEVELOPER CALLABLE FUNCTION - sets what's shown to the user 
//   when they try to navbar-click leave a page with unsaved changes
// PARAMS:
//    msg_str - Displays this message to user when they try to leave without saving
//    bool_add_default_howto - If true, appends the default howto string of clicking OK or Cancel
//       (" Click OK to continue (and lose changes), or Cancel to remain on this page (with unsaved changes)")
function navbar_set_unsaved_changes_message(msg_str, bool_add_default_howto)
{
    if (bool_add_default_howto) {
        navbar__unsaved_changes_message = msg_str + " Click OK to continue (and lose changes), or Cancel to remain on this page (with unsaved changes)";
    } else {
        navbar__unsaved_changes_message = msg_str;
    }
}
// DEVELOPER CALLABLE FUNCTION - Disables/Re-Enables navbar, and
//    optionally sets the message displayed to user when they click
//    on the disabled navbar.
//    (Pass false or empty string as 2nd arg to not overwrite default message)
function navbar_set_navbar_disabled(bool_value, optional_disabled_message)
{
    navbar__the_navbar_is_disabled = bool_value;
    if (optional_disabled_message) {
        navbar__disabled_navbar_message = optional_disabled_message;
    }
}
</script>

##EOFEOFEOF1
