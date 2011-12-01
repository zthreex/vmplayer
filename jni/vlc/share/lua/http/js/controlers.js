var current_id	=	1;
var currentArt	=	null;
var current_que	=	'main';
function updateStatus(){
	$.ajax({
		url: 'requests/status.xml',
		success: function(data,status,jqXHR){
			if(current_que=='main'){
				$('.dynamic').empty();
				$('#mediaTitle').append($('[name="filename"]',data).text());
				$('#totalTime').append(format_time($('length',data).text()));
				$('#currentTime').append(format_time($('time',data).text()));
				$('#seekSlider').slider({value: ($('time',data).text()/$('length',data).text()*100) })
				$('#currentVolume').append(Math.round($('volume',data).text()/5.12)+'%');
				$('#volumeSlider').slider({value: ($('volume',data).text()/5.12) });
				$('#rateSlider').slider({value: ($('rate',data).text()) });
				$('#currentRate').append(Math.round($('rate',data).text()*100)/100+'x');
				$('#audioSlider').slider({value: ($('audiodelay',data).text()) });
				$('#currentAudioDelay').append(Math.round($('audiodelay',data).text()*100)/100+'s');
				$('#subtitleSlider').slider({value: ($('subtitledelay',data).text()) });
				$('#currentSubtitleDelay').append(Math.round($('subtitledelay',data).text()*100)/100+'s');
				$('#seekSlider').attr('totalLength',$('length',data).text());
				$('#buttonPlay').attr('state',$('state',data).text());
				$('#buttonPlay').attr('mrl',$('[name="filename"]',data).text());
				if($('state',data).text()=='playing'){
					$('#buttonPlay').css({
						'background-image':'url("images/button_pause-48.png")'
					});
				}else{
					$('#buttonPlay').css({
						'background-image':'url("images/button_play-48.png")'
					});
				}
				if($('random',data).text()=='true'){
					$('#buttonShuffle').removeClass('ui-state-default');
					$('#buttonShuffle').addClass('ui-state-active');
				}else{
					$('#buttonShuffle').addClass('ui-state-default');
					$('#buttonShuffle').removeClass('ui-state-active');
				}
				if($('loop',data).text()=='true'){
					$('#buttonLoop').removeClass('ui-state-default');
					$('#buttonLoop').addClass('ui-state-active');
				}else{
					$('#buttonLoop').addClass('ui-state-default');
					$('#buttonLoop').removeClass('ui-state-active');
				}
				if($('repeat',data).text()=='true'){
					$('#buttonRepeat').removeClass('ui-state-default');
					$('#buttonRepeat').addClass('ui-state-active');
				}else{
					$('#buttonRepeat').addClass('ui-state-default');
					$('#buttonRepeat').removeClass('ui-state-active');
				}
				if($('[name="artwork_url"]',data).text()!=currentArt){
					var tmp	=	new Date();
					$('#albumArt').attr('src','/art?'+tmp.getTime());
					currentArt	=	$('[name="artwork_url"]',data).text();
					$('#albumArt').css({
						'visibility':'visible',
						'display':'block'
					});
				}else if($('[name="artwork_url"]',data).text()==""){
					$('#albumArt').attr('src','images/vlc-48.png');
				}
				if(pollStatus){
					setTimeout( updateStatus, 1000 );
				}
				
			}
			$('band',data).each(function(){
				var id		=	$(this).attr('id');
				var freq	=	convertHz($(this).attr('freqency'));
				var value	=	$(this).text() ? $(this).text() : 0;
				if(!$('#eq_container'+id).length){
					$('#window_equalizer').append('<div style="float:left;width:44px;" align="center" id="eq_container'+id+'"><div id="eq'+id+'_txt">'+value+'dB</div><div class="eqBand" id="eq'+id+'" style="font-size: 18px;"></div><div>'+freq+'</div></div>');
					$('#eq'+id).slider({
						min: -20,
						max: 20,
						step: 0.1,
						range: "min",
						value: value,
						animate: true,
						orientation: "vertical",
						stop: function(event,ui){
							$('#'+$(this).attr('id')+'_txt').empty().append(ui.value+'dB');
							sendCommad({
								command:'equalizer',
								val: ui.value,
								band: $(this).attr('id').substr(2)
							})
						},
						slide: function(event,ui){
							$('#'+$(this).attr('id')+'_txt').empty().append(ui.value+'dB');
						}
					});
				}else{
					$('#eq'+id).slider({value:value});
					$('#eq'+id+'_txt').empty().append(Math.round(value*100)/100+'dB');
				}
			});
			$('#preamp').slider('value',$('preamp',data).text());
			$('#preamp_txt').empty().append(Math.round($('preamp',data).text()*100)/100+'dB');
		},
		error: function(jqXHR,status,error){
			setTimeout( updateStatus, 500 );
		}
	});
}
function updatePlayList(){
	$('#libraryTree').jstree('refresh',-1);
}
function sendCommand(params,append){
	if(current_que=='stream'){
		$.ajax({
			url: 'requests/status.xml',
			data: params,
			success:function(data,status,jqXHR){
				if(append!=undefined){
					eval(append);
				}
				updateStatus();
				updatePlayList();
			}
		});
	}else{
		if(params.plreload===false){
			$.ajax({
				url: 'requests/status.xml',
				data: params,
				success:function(data,status,jqXHR){
					if(append!=undefined){
						eval(append);
					}
				}
			});
		}else{
			$.ajax({
				url: 'requests/status.xml',
				data: params,
				success:function(data,status,jqXHR){
					if(append!=undefined){
						eval(append);
					}
					updatePlayList();
				}
			});
		}	
	}
}
function browse(dir){
	dir	=	dir==undefined ? '~' : dir;
	$.ajax({
		url: 'requests/browse.xml',
		data:'dir='+encodeURIComponent(dir),
		success: function(data,status,jqXHR){
			var tgt	=	browse_target.indexOf('__')==-1 ? browse_target : browse_target.substr(0,browse_target.indexOf('__'));
			$('#browse_elements').empty();
            $('element',data).each(function(){
				if($(this).attr('type')=='dir' || $.inArray($(this).attr('name').substr(-3),video_types)!=-1 || $.inArray($(this).attr('name').substr(-3),audio_types)!=-1){
					$('#browse_elements').append(createElementLi($(this).attr('name'),$(this).attr('type'),$(this).attr('path'),$(this).attr('name').substr(-3)));
				}
			});
			$('[opendir]').dblclick(function(){
				browse($(this).attr('opendir'));
			});
			$('[openfile]').dblclick(function(){
				switch(tgt){
					case '#stream_input':
						$(browse_target).val($(this).attr('openfile'));
						break;
					case '#mosaic_open':
						$('li',browse_target).remove();
						$(browse_target).append(this);
						$(this).css({
							'margin-left' : -40,
							'margin-top' : -46,
							'float' : 'left'
						});
						break;
					case '#mobile':
						break;
					default:
						sendCommand('command=in_play&input=file://'+encodeURIComponent($(this).attr('openfile')));
						break;
				}
				$('#window_browse').dialog('close');
			});
			$('[opendir]').click(function(){
				switch(tgt){
					case '#mobile':
						browse($(this).attr('opendir'));
						break;
					default:
						break;
				}
			});
			$('[openfile]').click(function(){
				switch(tgt){
					case '#mobile':
						sendCommand('command=in_play&input=file://'+encodeURIComponent($(this).attr('openfile')),"window.location='mobile.html'");
						break;
					default:
						break;
				}
			});
			switch(tgt){
				case '#mobile':
					break;
				default:
					$('[selectable]').selectable();
					break;
			}
		},
		error: function(jqXHR,status,error){
			setTimeout('browse("'+dir+'")',1041);
		}
	});
}
function updateStreams(){
	$.ajax({
		url: 'requests/vlm.xml',
		success:function(data,status,jqXHR){
			$('#stream_info').accordion("destroy");
			$('#stream_info').empty();
			$('broadcast',data).each(function(){
				var stream_div	=	$('#stream_status_').clone();
				var name		=	$(this).attr('name');
				var loop		=	$(this).attr('loop')=='yes';
				var playing		=	$('instance',$(this)).attr('state')=='playing';
				var file		=	$('input',$(this)).text();
				var output		=	$('output',$(this)).text();
				var time		=	isNaN(Math.round($('instance',$(this)).attr('time')/1000000)) ? 0 : Math.round($('instance',$(this)).attr('time')/1000000);
				var length		=	isNaN(Math.round($('instance',$(this)).attr('length')/1000000)) ? 0 : Math.round($('instance',$(this)).attr('length')/1000000);
				$('[id]',stream_div).each(function(){
					$(this).attr('id',$(this).attr('id')+name);
				});
				$(stream_div).attr('id',$(stream_div).attr('id')+name);
				$('#stream_title_'+name,stream_div).append(name);
				$('#stream_file_'+name,stream_div).append(file);
				$('#stream_pos_'+name,stream_div).slider({
					value: 0,
					range: "min",
					min: 0,
					slide: function( event, ui ) {
						$( "#stream_current_time_"+name,stream_div ).empty();
						$( "#stream_current_time_"+name,stream_div ).append( format_time(ui.value) );
						$( "#stream_total_time_"+name,stream_div ).empty();
						$( "#stream_total_time_"+name,stream_div ).append( format_time($('#stream_pos_'+name,stream_div).slider('option','max')) );
						sendVLMCmd('control '+name+' seek '+Math.round(ui.value/$('#stream_pos_'+name,stream_div).slider('option','max')*100));
					},
					change: function(event, ui){
						$( "#stream_current_time_"+name,stream_div ).empty();
						$( "#stream_current_time_"+name,stream_div ).append( format_time(ui.value) );
						$( "#stream_total_time_"+name,stream_div ).empty();
						$( "#stream_total_time_"+name,stream_div ).append( format_time($('#stream_pos_'+name,stream_div).slider('option','max')) );
					}
				});
				$('#button_stream_stop_'+name,stream_div).click(function(){
					sendVLMCmd('control '+name+' stop');
					return false;
				});
				$('#button_stream_play_'+name,stream_div).click(function(){
					if($('span',this).hasClass('ui-icon-pause')){
						sendVLMCmd('control '+name+' pause');
					}else{
						sendVLMCmd('control '+name+' play');
					}
				});
				$('#button_stream_loop_'+name,stream_div).click(function(){
					if(loop){
						sendVLMCmd('setup '+name+' unloop');
					}else{
						sendVLMCmd('setup '+name+' loop');
					}
				});
				$('#button_stream_delete_'+name,stream_div).click(function(){
					sendVLMCmd('del '+name);
				});
				$('#stream_pos_'+name,stream_div).slider({
					max: length,
					value: time
				});
				if(playing){
					$('span',$('#button_stream_play_'+name,stream_div)).removeClass('ui-icon-play');
					$('span',$('#button_stream_play_'+name,stream_div)).addClass('ui-icon-pause');
				}
				if(loop){
					$('#button_stream_loop_'+name,stream_div).addClass('ui-state-active');
				}
				$(stream_div).css({
					'visibility':'',
					'display':''
				});
				$('#stream_info').append(stream_div);

			});
			$('.button').hover(
				function() { $(this).addClass('ui-state-hover'); },
				function() { $(this).removeClass('ui-state-hover'); }
			);
			$('#stream_info').accordion({
				header: "h3",
				collapsible: true,
				autoHeight: true
			});
			if(current_que=='stream'){
				$('.dynamic').empty();
				$('#mediaTitle').append($('[name="Current"] input',data).text());
				$('#totalTime').append(format_time(isNaN($('[name="Current"] instance',data).attr('length')) ? 0 : $('[name="Current"] instance',data).attr('length')/1000000));
				$('#currentTime').append(format_time(isNaN($('[name="Current"] instance',data).attr('time')) ? 0 : $('[name="Current"] instance',data).attr('time')/1000000));
				$('#seekSlider').slider({value: (($('[name="Current"] instance',data).attr('time')/1000000)/($('[name="Current"] instance',data).attr('length')/1000000)*100) });
				$('#seekSlider').attr('totalLength',$('[name="Current"] instance',data).attr('length')/1000000);
				$('#buttonPlay').attr('state',$('[name="Current"] instance',data).length>0 ? $('[name="Current"] instance',data).attr('state') : 'stopped');
				if($('[name="Current"] instance',data).attr('state')=='playing'){
					$('#buttonPlay').css({
						'background-image':'url("images/button_pause-48.png'
					});
				}else{
					$('#buttonPlay').css({
						'background-image':'url("images/button_play-48.png'
					});
				}
				setTimeout( updateStreams, 1000 );
			}
		}
	});
}
function updateEQ(){
	$.ajax({
		url: 'requests/equalizer.xml',
		success: function(data,status,jqXHR){
			$('band',data).each(function(){
				var id		=	$(this).attr('id');
				var freq	=	convertHz($(this).attr('freqency'));
				var value	=	$(this).text() ? $(this).text() : 0;
				if(!$('#eq_container'+id).length){
					$('#window_equalizer').append('<div style="float:left;width:44px;" align="center" id="eq_container'+id+'"><div id="eq'+id+'_txt">'+value+'dB</div><div class="eqBand" id="eq'+id+'" style="font-size: 18px;"></div><div>'+freq+'</div></div>');
					$('#eq'+id).slider({
						min: -20,
						max: 20,
						step: 0.1,
						range: "min",
						value: value,
						animate: true,
						orientation: "vertical",
						stop: function(event,ui){
							$('#'+$(this).attr('id')+'_txt').empty().append(ui.value+'dB');
							sendEQCmd({
								command:'equalizer',
								val: ui.value,
								band: $(this).attr('id').substr(2)
							})
						},
						slide: function(event,ui){
							$('#'+$(this).attr('id')+'_txt').empty().append(ui.value+'dB');
						}
					});
				}else{
					$('#eq'+id).slider({value:value});
					$('#eq'+id+'_txt').empty().append(Math.round(value*100)/100+'dB');
				}
			});
			$('#preamp').slider('value',$('preamp',data).text());
			$('#preamp_txt').empty().append(Math.round($('preamp',data).text()*100)/100+'dB');
		}
	})
}
function sendVLMCmd(command,append){
	var commands	=	command.split(';');
	if(commands.length>1){
		sendBatchVLMCmd(command,append);
	}else{
		if(current_que=='main'){
			$.ajax({
				url: 'requests/vlm_cmd.xml',
				data: 'command='+encodeURIComponent(command),
				success: function(data,status,jqXHR){
					if($('error',data).text()){
						$('#error_container').append('<div>'+$('error',data).text()+'</div>');
						$('#window_error').dialog('open');
					}
					if(append!=undefined){
						eval(append);
					}
					updateStreams();
				}
			});
		}else{
			$.ajax({
				url: 'requests/vlm_cmd.xml',
				data: 'command='+encodeURIComponent(command),
				success: function(data,status,jqXHR){
					if($('error',data).text()){
						$('#error_container').append('<div>'+$('error',data).text()+'</div>');
						$('#window_error').dialog('open');
					}
					updateStreams();
				}
			});
		}
	}
}
function sendBatchVLMCmd(command,append){
	var commands	=	command.split(';');
	$.ajax({
		url: 'requests/vlm_cmd.xml',
		data: 'command='+encodeURIComponent(commands.shift()),
		success:function(data,status,jqXHR){
			if($('error',data).text()){
				$('#error_container').append('<div>'+$('error',data).text()+'</div>');
				$('#window_error').dialog('open');
			}
			sendVLMCmd(commands.join(';'),append);
		}
	});
}
function sendEQCmd(params){
	$.ajax({
		url: 'requests/equalizer.xml',
		data: params,
		success: function(data,status,jqXHR){
			updateEQ();
		}
	});
}

$(function(){
	$('#libraryTree').jstree({
		"xml_data":{
			"ajax":{
				"url" : "requests/playlist_jstree.xml"
			},
			"xsl" : "nest"
		},
		"themeroller":{
			"item_leaf":"ui-icon-video"
		},
		"plugins" : ["xml_data","ui","themeroller"]
	}).bind("loaded.jstree", function (event, data) {
		$('[current]','[id^="plid_"]').each(function(){
			$(this).addClass('ui-state-highlight');
			current_id	=	$(this).attr('id').substr(5);
		});
	}).bind("refresh.jstree",function(event,data){
		$('[current]','[id^="plid_"]').each(function(){
			$(this).addClass('ui-state-highlight');
			current_id	=	$(this).attr('id').substr(5);
		});
	});
	updateStatus();
	updateStreams();
	updateEQ();
});
