(function ($) {
    Drupal.behaviors.idestination_comment_tag = {
        attach: function (context, settings) {
            if (!window.location.origin) {
                var base_url_all = window.location.protocol + "//" + window.location.hostname + (window.location.port ? ':' + window.location.port : '');  //this code is for IE where window.location.origin is not work
            } else {
                var base_url_all = window.location.origin;
            }
            var a = base_url_all;
            
            var start = /@/ig; // @ Match
            var word = /@(\w+)/ig; //@abc Match
            var drop_down = jQuery("#typeahead-dropdown");
            jQuery("#edit-comment-body-und-0-value").once().keyup(function () {
                var content = jQuery(this).val(); //Content Box Data
                var go = content.match(start); //find Content Matching @
                var name = content.match(word); //Content Matching @abc
                                
//                if (name != null && name.length > 0)
//                {   
//                    var tmp_go = go.toString();
//                    var res_go = tmp_go.split(",");
//                    
//                    var tmp = name.toString();
//                    var res = tmp.split(",");
//                    name = res[res_go.length - 1];
//                    console.log('res = '+res[res_go.length - 1]);
//                }
                
                var dataString = 'count=10&searchword=' + name;
                //If @ available
                if (go != null && go.length > 0) {
                    //if @abc avalable
                    if (name != null && name.length > 0)
                    {
                        jQuery.ajax({
                            type: "GET",
                            url: a + "/typehead/json", // Database name search 
                            data: dataString,
                            cache: false,
                            success: function (data)
                            {
                                drop_down.html(data).show();
                            }
                        });
                    }
                    else {
                        drop_down.html('').hide();
                    }
                }
            });
            jQuery(document).on("click", ".js-nav", function () {
                var first_name = jQuery(this).attr('data-fn');
                var last_name = jQuery(this).attr('data-ln');
                var search_key = jQuery(this).attr('data-search-query');
                var old = jQuery("#edit-comment-body-und-0-value").val();
                var content = old.replace('@' + search_key, '#' + first_name + '_' + last_name + ' ');
                jQuery("#edit-comment-body-und-0-value").val(content).focus();
                drop_down.html('').hide();
            });
        }
    };
})(jQuery);

