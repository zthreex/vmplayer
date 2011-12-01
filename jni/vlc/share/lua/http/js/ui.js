$(function(){
	$( "#seekSlider" ).slider({
		range: "min",
		value: 0,
		min: 0,
		max: 100,
		stop: function( event, ui ) {
			$( "#currentTime" ).empty().append( format_time(Math.round((ui.value/100)*$('#seekSlider').attr('totalLength'))) );
			switch(current_que){
				case 'main':
					sendCommand({'command':'seek','val':Math.round((ui.value/100)*$('#seekSlider').attr('totalLength'))});
					break;
				case 'stream':
					sendVLMCmd('control Current seek '+ui.value);
					break;
			}
		}
	});
	$( "#volumeSlider" ).slider({
		range: "min",
		value: 50,
		min: 0,
		max: 100,
		stop: function( event, ui ) {
			$( "#currentVolume" ).empty().append( ui.value+"%" );
			sendCommand({
				'command':'volume',
				'val':Math.round(ui.value*5.12)
			})
		}
	});
	$('#buttonStop').click(function(){
		switch(current_que){
			case 'main':
				sendCommand({'command' : 'pl_stop'})
				break;
			case 'stream':
				sendVLMCmd('control Current stop');
				break;
		}
		return false;
	});
	$('#buttonPlay').click(function(){
		if($(this).attr('state')=='stopped'){
			switch(current_que){
				case 'main':
					var id	=	$('.jstree-clicked','#libraryTree').length>0 ?  $('.jstree-clicked','#libraryTree').first().parents().first().attr('id').substr(5) : current_id;
					sendCommand({'command':'pl_play','id':id});
					break;
				case 'stream':
					sendVLMCmd('control Current play');
					flowplayer("player", "http://releases.flowplayer.org/swf/flowplayer-3.2.7.swf");
					break;
			}
		}else{
			switch(current_que){
				case 'main':
					sendCommand({'command':'pl_pause'});
					break;
				case 'stream':
					sendVLMCmd('control Current pause');
					break;
			}
		}
		return false;
	});
	$('#buttonFull').click(function(){
		sendCommand({'command':'fullscreen'});
		return false;
	});
	$('#stream_host').val(stream_server);
})