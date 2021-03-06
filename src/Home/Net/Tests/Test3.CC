/*methods:
TreeNodeInit
TreeNodeParamInit
TreeNodeAdd
TreeNodeParamAdd
TreeNodeFind
TreeNodeQueueInit
TreeNodeQueueAdd
TreeNodeQueueFind*/

// Binary Tree Circular Queues.

// To Create, CAlloc new CTreeNode.

// To Insert, CAlloc new CTreeNode, fill value member,
// then traverse the tree based on the value members of
// the tree. If new val is above current entry val, go right.
// if get NULL, insert there. else, continue traversal,
// if new val below current entry val, go left.

// if same , just go right idek. lol gotta do something.

/*class CTreeQueue:CQueue
{
	I64 whatever_data; // arbitrary filler class. Importantly, implements CQueue.
};*/

class CTreeQueue
{
	CTreeQueue *next;
	CTreeQueue *last;

	I64 whatever_data; // arbitrary filler class. Importantly, implements CQueue.
};


class CTreeNode
{
	I64 value;

	CTreeNode *left;
	CTreeNode *right;

	CTreeQueue* queue;
};


CTreeNode *TreeNodeInit()
{ // init new empty tree/node.
	CTreeNode *tree_node = CAlloc(sizeof(CTreeNode));

	return tree_node;
}

U0 TreeNodeAdd(CTreeNode *node, CTreeNode *tree)
{
	// using temp and last allows avoiding recursion and non-growing stack issues.
	CTreeNode *temp_tree = tree;
	CTreeNode *last_tree = temp_tree;

	while (temp_tree)
	{
		if (node->value < temp_tree->value)
		{ // if node smaller, go left
			last_tree = temp_tree;
			temp_tree = temp_tree->left;
		}
		else
		{ // if node equal or larger, go right
			last_tree = temp_tree;
			temp_tree = temp_tree->right;
		}
	}
	// at the end of this, this _should_ result in last_tree
	// being the resulting tree to store the node inside of. i guess recompute the direction and set.

	if (node->value < last_tree->value)// if node smaller, go left
		last_tree->left = node;
	else // if node equal or larger, go right
		last_tree->right = node;
}

CTreeNode *TreeNodeParamAdd(I64 node_value, CTreeNode *tree)
{ // add a node using params, return pointer to the node
	CTreeNode *result = TreeNodeInit;
	result->value = node_value;

	TreeNodeAdd(result, tree);

	return result;
}

CTreeNode *TreeNodeParamInit(I64 value)
{
	CTreeNode *result = TreeNodeInit;
	result->value = value;

	return result;
}

CTreeNode *TreeNodeFind(I64 value, CTreeNode *tree)
{
	CTreeNode *temp_tree = tree;

	while (temp_tree)
	{
		if (value < temp_tree->value) // if value smaller, go left
			temp_tree = temp_tree->left;
		else if (value > temp_tree->value) // if value larger, go right
			temp_tree = temp_tree->right;
		else // if value equal, match! i guess?
			break;
	}

	return temp_tree; // ! NULL if not found.
}

U0 TreeNodeQueueInit(CTreeNode *node)
{
	node->queue = CAlloc(sizeof(CTreeQueue));
	QueueInit(node->queue);
}

U0 TreeNodeQueueAdd(I64 some_data, CTreeNode *node)
{
	CTreeQueue *new_entry;

	if (!node->queue)
	{
		TreeNodeQueueInit(node);
		node->queue->whatever_data = some_data;
	}
	else
	{
		new_entry = CAlloc(sizeof(CTreeQueue));
		QueueInit(new_entry);
		new_entry->whatever_data = some_data;
		QueueInsert(new_entry, node->queue->last);
	}
	
}

CTreeQueue *TreeNodeQueueFind(I64 some_data, CTreeNode *node)
{

	CTreeQueue *temp_queue;

	if (node->queue)
	{

		if (node->queue->whatever_data == some_data)
			return node->queue;

		temp_queue = node->queue->next;
		while (temp_queue != node->queue)
		{
			if (temp_queue->whatever_data == some_data)
				return temp_queue;
			temp_queue = temp_queue->next;
		}
	}

	return NULL;
}


// so way i take it, is that UDP binary tree will
// do a Find based on port, and then the CQueue on
// that port node will be where to look for/add an
// address to the address circular Queue.


// udp bound socket list would now be a
// a tree. instead of doing udp_bound[port] to
// get the CUDP socket, that would just give you
// the node that has the Queue of sockets, per
// the address.























"Head\n";
CTreeNode *tree = TreeNodeInit;
tree->value = 15;

//ClassRep(tree);

"left of Head\n";
CTreeNode *node = TreeNodeInit; 
node->value = 14;
TreeNodeAdd(node, tree);

"right of Head\n";
node = TreeNodeInit;
node->value = 20;
TreeNodeAdd(node, tree);

"right of right of Head\n";
node = TreeNodeInit; // right of right of head
node->value = 25;
TreeNodeAdd(node, tree);

"left of right of head\n";
node = TreeNodeInit;
node->value = 15; // left of right of head
TreeNodeAdd(node, tree);

ClassRep(tree);
ClassRep(tree->left);
ClassRep(tree->right);
ClassRep(tree->right->right);
ClassRep(tree->right->left);






$SP,"<1>",BI=1$













































              �      �   3   �   N   �   e   �   |   �   �   b   �   �   �   =   �   c   �   �   �   �   �   �   �   �   �   �   �   �   �   �   s   �   �   �   �     �     �   $  �   �   @     V     ]   7  u   m  {   �  �   X  �   d  �   �  �   �  �   l  �   �  �   �   =   �   H   �   r   �   x   �   �   �   �   g   �   b   �   �   m   �   s   �   �   �   �   �   �   �   �     �   
  �   �   �   �   �   �   �   �   �   �   =   �   G     Z     l   ?  s   ^  �   e  �   ^  �   |  �     �   �  �   x  �   �   @   	   �   s      �   �   	   z   �      Y        �        �        �   �   	   �   �   	   �   �        �      �   c   	   ,  �      r  �      U  �      �  �   	   z  �      �   5   �   >   �   C   �   M   �   s   �   ~   g  �   o  �   v  �   z  �   �  �   �  �   y    }    k  �   p    �  �   �  �   �  �   �  �   �  �   �  �   X  �   a  �   F  �   R  �   +  �   3  �   �   d   �   o   �   �   �   �   �   �   �   �   �   �   �   �   �   �   �   �   |   �   �   �   \   
  b     �     �     �   	  �       �     �   �   �   �   �   n  �   t  �   �   ^   �   c      |   %  �   �   �   �   �   �   �   �   �    