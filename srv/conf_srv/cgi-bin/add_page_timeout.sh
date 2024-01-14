#
# Use this script to inject a session timeout warning to a webgui page.  When
#    the timeout nears, a modal warning pops up on the screen, prompting the
#    user to click anywhere to refresh the page (and the session).  If the
#    user fails to respond, the user is redirected to the timeout page,
#    letting them know they're logged out, saving them from being
#    redirected to the timeout page when they next tried to use the webgui.
#
# INSTRUCTIONS:
#    This script should be sourced, from inside other cgi-bin scripts, once the
#    <body> has been output, preferably near the start of the page's body output
#
# NOTE: The webgui session timeout is performed on the server.  This script
#       creates a client-side timer to warn about the server timeout, but has
#       no control over the server-side timer.
#
# NOTE: Even pages with form controls or xttp data queries, these are server
#       interactions that will not reset the SESSION_CHK timer, so these pages
#       should also include this script for the timeout warning.
#
# NOTE: This script has been tested on Chrome, Edge, and Firefox


# Session timeout.  This must match the SESSION_TIMEOUT #define value in conf_manager.c
SESSION_TIMEOUT_SECS=600
# However long user sees the timeout warning before they're moved to the timeout page
POPUP_WARNING_TIME_SECS=30


cat << '##EOFEOFEOF1'

<!-- Timeout Modal -->
<div id="timeoutModal" class="modal">

   <!-- Modal content -->
   <div class="modal-content">
     <div class="modal-header">
       <h2>Webgui Timeout Warning</h2>
     </div>
     <div class="modal-body">
       <p>You have been idle on this page for almost ten minutes, and will be logged out soon</p>
       <p>If you wish to remain logged in, please click anywhere on this page...</p>
     </div>
   </div>

</div>


<style>


/* The Modal (background) */
.modal {
  display: none; /* Hidden by default */
  position: fixed; /* Stay in place */
  z-index: 1; /* Sit on top */
  left: 0;
  top: 0;
  width: 100%; /* Full width */
  height: 100%; /* Full height */
  overflow: auto; /* Enable scroll if needed */
  background-color: rgb(0,0,0); /* Fallback color */
  background-color: rgba(0,0,0,0.4); /* Black w/ opacity */
}

/* Modal Content/Box */
.modal-content {
  background-color: #fefefe;
  margin: 15% auto; /* 15% from the top and centered */
  padding: 20px;
  border: 1px solid #888;
  width: 80%; /* Could be more or less, depending on screen size */
}

/* Modal Header */
.modal-header {
  padding: 2px 16px;
  background-color: #5cb85c;
  color: white;
  text-align: left;
}

/* Modal Body */
.modal-body {padding: 2px 16px;}

/* Modal Content */
.modal-content {
  position: relative;
  background-color: #fefefe;
  margin: auto;
  padding: 0;
  border: 1px solid #888;
  width: 80%;
  box-shadow: 0 4px 8px 0 rgba(0,0,0,0.2),0 6px 20px 0 rgba(0,0,0,0.19);
  animation-name: animatetop;
  animation-duration: 0.4s
}

/* Add Animation */
@keyframes animatetop {
  from {top: -300px; opacity: 0}
  to {top: 0; opacity: 1}
}

</style>

##EOFEOFEOF1

# Convert our shell variables to values javascript can use
echo '<script>'
echo 'var popup_interval_msec = ' $POPUP_WARNING_TIME_SECS ' * 1000;'
echo 'var timeout_interval_msec = ' $SESSION_TIMEOUT_SECS ' * 1000 - popup_interval_msec;'
echo '</script>'



cat << '##EOFEOFEOF2'

<script>

// Get the modal
var modal = document.getElementById("timeoutModal");

// When the timeout expires, pop up the modal
var idleTimer = window.setInterval(pop_up_warning_modal, timeout_interval_msec);
// When the model pops up, this holds the timer for going to the timeout html page
var timeoutTimer = 0;

function pop_up_warning_modal()
{
  modal.style.display = "block";
  clearInterval(idleTimer);
  timeoutTimer = window.setInterval(go_to_timeout_page, popup_interval_msec);
}

// If the modal is up, if the user clicks anywhere, close modal & refresh the page
window.onclick = function(event) {
    if (modal.style.display == "block") {
        modal.style.display = "none";
        // reload the page which refreshes the server's login timeout
        location.reload();
    }
}

// User has idle'd out .. head to timeout page
function go_to_timeout_page() {
   window.location.href = '/rsu-timeout.html';
}
      
</script>

##EOFEOFEOF2
